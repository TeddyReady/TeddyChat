#include "clientwindow.h"
#include "ui_clientwindow.h"

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientWindow)
{
    //Загружаем настройки
    settings = new QSettings("/home/kataich75/qtprojects/TECH/TeddyClient/settings.ini", QSettings::IniFormat, this);
    uploadSettings();

    ui->setupUi(this);
    ui->disconnectAct->setDisabled(true);
    ui->sendButton->setDisabled(true);
    ui->saveHistoryAct->setDisabled(true);
    ui->labelIP->setText(client.ip);
    ui->labelPort->setText(QString::number(client.port));
    switch(client.status){
    case Status::Online:
        ui->labelStatus->setText("Online");
        ui->actionOnline->setChecked(true);
        break;
    case Status::NotInPlace:
        ui->labelStatus->setText("Not In Place");
        ui->actionNotInPlace->setChecked(true);
        break;
    case Status::NotDisturb:
        ui->labelStatus->setText("Do Not Disturb");
        ui->actionDoNotDisturb->setChecked(true);
        break;
    }

    client.socket = new QSslSocket(this);
    connect(client.socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(client.socket, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(client.socket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
    connect(client.socket, SIGNAL(encrypted()), this, SLOT(slotEncrypted()));
    setWindowTitle(client.username);
    dataSize = 0;

    //Контекстное меню списка клиентов
    ui->clientList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->clientList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    //Контекстное меню на кнопке
    btnMenu = new QMenu;

    sendMsg = new QAction("..message");
    sendMsg->setCheckable(true);
    sendMsg->setChecked(true);
    connect(sendMsg, SIGNAL(triggered(bool)), this, SLOT(slotSendMessage(bool)));

    sendPic = new QAction("..image");
    sendPic->setCheckable(true);
    sendPic->setDisabled(true);
    sendPic->setChecked(false);
    connect(sendPic, SIGNAL(triggered(bool)), this, SLOT(slotSendPicture(bool)));

    btnMenu->addAction(sendMsg);
    btnMenu->addAction(sendPic);
}

ClientWindow::~ClientWindow()
{
    saveSettings();
    file.close();
    delete settings;
    delete ui;
}

void ClientWindow::uploadSettings(){
    client.ip = settings->value("ip", "127.0.0.1").toString();
    client.port = settings->value("port", "45678").toInt();
    client.username = settings->value("username", "Unknown User").toString();
    client.status = settings->value("status", Status::Online).toInt();
    setGeometry(settings->value("geometry", QRect(200, 200, 530, 388)).toRect());
}
void ClientWindow::saveSettings(){
    settings->setValue("ip", client.ip);
    settings->setValue("port", client.port);
    settings->setValue("username", client.username);
    settings->setValue("status", client.status);
    settings->setValue("geometry", geometry());
}

void ClientWindow::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::RightButton)
        ui->sendButton->setMenu(btnMenu);
    else ui->sendButton->setMenu(NULL);
}

void ClientWindow::sendToServer(int command, QString message)
{
    QByteArray data; data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << (quint16)0 << (quint8)command;
    if(command == Commands::SendMessage) {
        out << client.username << message;
    } else if (command == Commands::Authentication) {
        out << client.ip << QString::number(client.port)
            << client.username << QString::number(client.status)
            << client.date << client.time;
    } else if (command == Commands::Exit) {
        out << client.username;
    } else if (command == Commands::UpdateDataBase) {}
    out.device()->seek(0);
    out << (quint16)(data.size() - sizeof(quint16));
    client.socket->write(data);
}
void ClientWindow::slotReadyRead()
{
    //PACKET_DEFENCER
    QDataStream in(client.socket);
    if (dataSize == 0) {
        if (client.socket->bytesAvailable() < (int)sizeof(quint16)) return;
        in >> dataSize;
    }
    if (client.socket->bytesAvailable() < dataSize)
        return;
    else dataSize = 0;
    //END
    quint8 command;
    in >> command;
    if((int)command == Commands::SendMessage) {
        QString name, message, date, time;
        in >> name >> message >> date >> time;
        if(name == client.username) {            
            ui->incomingField->setTextColor(Qt::black);
            ui->incomingField->append("You: " + message);
        } else {
            if (client.status != Status::NotDisturb){
                QSound::play("/home/kataich75/qtprojects/TECH/TeddyClient/newmessage.wav");
            }

            ui->incomingField->setTextColor(Qt::blue);
            ui->incomingField->append(name + ": " + message);
        }
        //Добавляем данные для XML
        xmlData.push_back(date);
        xmlData.push_back(time);
        xmlData.push_back("127.0.0.1");
        xmlData.push_back(name);
        xmlData.push_back(message);

    } else if ((int)command == Commands::Authentication) {
        QString name, message;
        in >> name >> message;
        if(name == client.username) {
            ui->incomingField->setTextColor(Qt::darkGreen);
            ui->incomingField->append(message);
            ui->saveHistoryAct->setEnabled(true);
            sendToServer(Commands::UpdateDataBase);
        } else {
            ui->incomingField->setTextColor(Qt::green);
            ui->incomingField->append(message);
        }
    } else if ((int)command == Commands::Exit) {
        QString name, message;
        in >> name >> message;
        if(name != client.username){
            ui->incomingField->setTextColor(Qt::darkRed);
            ui->incomingField->append(message);
            for(int i = 0; i < includedClients.size(); i++){
                if(name == ui->clientList->item(i)->text()){
                    includedClients.removeAt(i);
                    QListWidgetItem *it = ui->clientList->takeItem(i);
                    delete it;
                    break;
                }
            }
        }
    } else if ((int)command == Commands::UpdateDataBase) {
        quint8 countOfClients;
        in >> countOfClients;
        for (int i = 0; i < (int)countOfClients; i++) {
            QString name, status, date, time;
            in >> name >> status >> date >> time;
            ui->clientList->addItem(name);
            MyClient *ptr = new MyClient("127.0.0.1", 45678, name, status.toInt(), date, time, nullptr);
            includedClients.push_back(ptr);
        }
    } else if ((int)command == Commands::NewClient) {
        QString name, status, date, time;
        in >> name >> status >> date >> time;
        ui->clientList->addItem(name);
        MyClient *ptr = new MyClient("127.0.0.1", 45678, name, status.toInt(), date, time, nullptr);
        includedClients.push_back(ptr);
    }
}

void ClientWindow::slotEncrypted(){
    client.date = QDate::currentDate().toString();
    client.time = QTime::currentTime().toString();
    sendToServer(Commands::Authentication);
}

//Меню "File"
void ClientWindow::on_connectAct_triggered()
{
    client.socket->setPeerVerifyMode(QSslSocket::VerifyNone);
    client.socket->connectToHostEncrypted(client.ip, client.port);
}
void ClientWindow::slotSocketConnected()
{
    ui->connectAct->setDisabled(true);
    ui->disconnectAct->setEnabled(true);
    ui->quitAct->setDisabled(true);
    ui->sendButton->setEnabled(true);
    ui->ipPortAct->setDisabled(true);
    ui->nameAct->setDisabled(true);
}

void ClientWindow::on_disconnectAct_triggered()
{
    sendToServer(Commands::Exit);
    client.socket->disconnectFromHost();
}
void ClientWindow::slotSocketDisconnected()
{
    ui->connectAct->setEnabled(true);
    ui->disconnectAct->setDisabled(true);
    ui->quitAct->setEnabled(true);
    ui->sendButton->setDisabled(true);
    ui->incomingField->clear();
    ui->ipPortAct->setEnabled(true);
    ui->nameAct->setEnabled(true);
    ui->clientList->clear();
}

void ClientWindow::on_saveHistoryAct_triggered()
{
    DialogXML *window = new DialogXML(this);
    window->setWindowTitle("Save your chat history");
    window->show();
    connect(window, SIGNAL(savePath(QString, quint64)), this, SLOT(slotSavePath(QString, quint64)));
}
void ClientWindow::slotSavePath(QString path, quint64 pass)
{
    if(path != ""){
        //Шифрование
        Cipher crypto;
        crypto.setKey(pass);
        //Формируем XML
        general = doc.createElement("history");
        for (int i = 0; i < xmlData.size(); i += 5){
            //Теги
            this->date = doc.createElement("date");
            this->time = doc.createElement("time");
            this->ip = doc.createElement("ip");
            this->name = doc.createElement("username");
            this->message = doc.createElement("message");
            doc.appendChild(general);
            //Сообщения между тегами
            QDomText date_t = doc.createTextNode(crypto.encryptToString(xmlData[i]));
            QDomText time_t = doc.createTextNode(crypto.encryptToString(xmlData[i+1]));
            QDomText ip_t = doc.createTextNode(crypto.encryptToString(xmlData[i+2]));
            QDomText name_t = doc.createTextNode(crypto.encryptToString(xmlData[i+3]));
            QDomText message_t = doc.createTextNode(crypto.encryptToString(xmlData[i+4]));
            //Добавление в XML
            general.appendChild(this->date);
            this->date.appendChild(date_t);
            general.appendChild(this->time);
            this->time.appendChild(time_t);
            general.appendChild(this->ip);
            this->ip.appendChild(ip_t);
            general.appendChild(this->name);
            this->name.appendChild(name_t);
            this->name.appendChild(this->message);
            this->message.appendChild(message_t);
        }
        //END
            file.setFileName(path);
            file.open(QIODevice::WriteOnly);
            QTextStream out(&file);
            doc.save(out, 4);
    }
}

void ClientWindow::on_quitAct_triggered()
{
    qApp->exit();
}

//Меню "Settings"
void ClientWindow::on_ipPortAct_triggered()
{
    DialogIPPort *window = new DialogIPPort(this, client.ip, client.port);
    window->setWindowTitle("Edit Your Ip and Port");
    window->show();
    connect(window, SIGNAL(dialogIPPortParams(QString,int)), this, SLOT(slotDialogIPPortParams(QString,int)));
}
void ClientWindow::slotDialogIPPortParams(QString ip, int port)
{
    client.ip = ip;
    client.port = port;
    ui->labelIP->setText(ip);
    ui->labelPort->setText(QString::number(port));
}

void ClientWindow::on_nameAct_triggered()
{
    DialogUserName *window = new DialogUserName(this, client.username);
    window->setWindowTitle("Edit Your Name");
    window->show();
    connect(window, SIGNAL(dialogUserNameParams(QString)), this, SLOT(slotDialogUserNameParams(QString)));
}
void ClientWindow::slotDialogUserNameParams(QString username)
{
    client.username = username;
    setWindowTitle(client.username);
}

void ClientWindow::on_actionOnline_triggered()
{
    if (ui->actionOnline->isChecked()) {
        ui->actionNotInPlace->setChecked(false);
        ui->actionDoNotDisturb->setChecked(false);
        client.status = Status::Online;
        ui->labelStatus->setText("Online");
    }
}
void ClientWindow::on_actionNotInPlace_triggered()
{
    if (ui->actionNotInPlace->isChecked()) {
        ui->actionOnline->setChecked(false);
        ui->actionDoNotDisturb->setChecked(false);
        client.status = Status::NotInPlace;
        ui->labelStatus->setText("Not In Place");
    }
}
void ClientWindow::on_actionDoNotDisturb_triggered()
{
    if (ui->actionDoNotDisturb->isChecked()) {
        ui->actionOnline->setChecked(false);
        ui->actionNotInPlace->setChecked(false);
        client.status = Status::NotDisturb;
        ui->labelStatus->setText("Do Not Disturb");
    }
}
//Меню "Help"
void ClientWindow::on_appAct_triggered()
{
    DialogAboutAutor *window = new DialogAboutAutor(this);
    window->setWindowTitle("About Application");
    window->show();
}

//UI Buttons
void ClientWindow::on_profileButton_clicked()
{
    DialogAboutClient *window = new DialogAboutClient(this, client);
    window->show();
}

void ClientWindow::slotSendMessage(bool)
{
    if(sendMsg->isChecked()){
        sendMsg->setChecked(true);
        sendPic->setChecked(false);
    } else sendMsg->setChecked(false);
}
void ClientWindow::slotSendPicture(bool) {
    if(sendPic->isChecked()){
        sendPic->setChecked(false);
        sendMsg->setChecked(false);
    } else sendPic->setChecked(true);
}

void ClientWindow::on_sendButton_clicked()
{
    if (sendMsg->isChecked()){
        sendToServer(Commands::SendMessage, ui->messageField->text());
        ui->messageField->clear();
    }
}
void ClientWindow::on_messageField_returnPressed()
{
    if (sendMsg->isChecked()){
        sendToServer(Commands::SendMessage, ui->messageField->text());
        ui->messageField->clear();
    }
}

//UI ClientList
void ClientWindow::showContextMenu(QPoint pos)
{
    if(!includedClients.isEmpty()){
        QMenu *menu = new QMenu(this);
        QAction *infoAct = new QAction("Info about...");
        connect(infoAct, SIGNAL(triggered()), this, SLOT(slotInfoAbout()));
        menu->addAction(infoAct);
        menu->popup(ui->clientList->viewport()->mapToGlobal(pos));
    }
}
void ClientWindow::slotInfoAbout(){
    for (int i = 0; i < includedClients.size(); i++){
        if (ui->clientList->currentItem()->text() == includedClients[i]->username) {
            DialogAboutClient *window = new DialogAboutClient(this, *includedClients[i]);
            window->setWindowTitle("About " + includedClients[i]->username);
            window->show();
            break;
        }
    }
}
