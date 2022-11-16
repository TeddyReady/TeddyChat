#include "clientwindow.h"
#include "ui_clientwindow.h"

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientWindow)
{
    ui->setupUi(this);
    //Загружаем настройки
    settings = new QSettings("/home/kataich75/qtprojects/TECH/TeddyClient/other/settings.ini", QSettings::IniFormat, this);
    uploadSettings();

    ui->chatList->setIconSize(QSize(320, 240));
    ui->clientList->setIconSize(QSize(60, 40));

    ui->disconnectAct->setDisabled(true);
    ui->sendButton->setDisabled(true);
    ui->saveHistoryAct->setDisabled(true);
    ui->labelIP->setText(client.ip);
    ui->labelPort->setText(QString::number(client.port));
    ui->actionOther->setText(client.statusName);
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
    case Status::Other:
        ui->labelStatus->setText(client.statusName);
        ui->actionOther->setChecked(true);
        break;
    }

    client.socket = new QSslSocket(this);
    connect(client.socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(client.socket, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(client.socket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
    connect(client.socket, SIGNAL(encrypted()), this, SLOT(slotEncrypted()));
    setWindowTitle(client.username);
    dataSize = 0;
    isConnected = false;

    //Контекстное меню списка клиентов
    ui->clientList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->clientList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    //Контекстное меню на поле сообщений
    ui->chatList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->chatList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenuOnMessageField(QPoint)));

    //Контекстное меню на кнопке
    ui->sendButton->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->sendButton, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenuOnSendButton(QPoint)));
}

ClientWindow::~ClientWindow()
{
    saveSettings();
    file.close();
    delete settings;
    delete ui;
}

void ClientWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    ui->quitAct->triggered();
}

void ClientWindow::uploadSettings(){
    client.ip = settings->value("ip", "127.0.0.1").toString();
    client.port = settings->value("port", "45678").toInt();
    client.username = settings->value("username", "Unknown User").toString();
    client.status = settings->value("status", Status::Online).toInt();
    client.statusName = settings->value("customStatus", "Working on Lab").toString();
    client.path = settings->value("avatarPath", ":/new/prefix1/other/client.png").toString();
    setGeometry(settings->value("geometry", QRect(200, 200, 530, 388)).toRect());
}
void ClientWindow::saveSettings(){
    settings->setValue("ip", client.ip);
    settings->setValue("port", client.port);
    settings->setValue("username", client.username);
    settings->setValue("status", client.status);
    settings->setValue("customStatus", client.statusName);
    settings->setValue("avatarPath", client.path);
    settings->setValue("geometry", geometry());
}

void ClientWindow::sendToServer(int command, QString message, int option)
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
            << client.date << client.time << client.path << client.statusName;
    } else if (command == Commands::Exit) {
        out << client.username;
    } else if (command == Commands::DataChanged) {
        out << client.username;
        if (option == Status::Other) {
            out << QString::number(option);
        } out << message;
    } else if (command == Commands::Image) {
        out << client.username << message;
    }
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
    if(static_cast<int>(command) == Commands::SendMessage) {
        QString name, message, date, time;
        in >> name >> message >> date >> time;
        if(name == client.username) {
            QListWidgetItem *item = new QListWidgetItem("You: " + message, ui->chatList);
            item->setForeground(Qt::black);
            ui->chatList->addItem(item);
        } else {
            if (client.status != Status::NotDisturb){
                QSound::play("/home/kataich75/qtprojects/TECH/TeddyClient/other/newmessage.wav");
            }
            QListWidgetItem *item = new QListWidgetItem(name + ": " + message, ui->chatList);
            item->setForeground(Qt::blue);
            ui->chatList->addItem(item);
        }
        //Добавляем данные для XML
        xmlData.push_back(date);
        xmlData.push_back(time);
        xmlData.push_back("127.0.0.1");
        xmlData.push_back(name);
        xmlData.push_back(message);

    } else if (static_cast<int>(command) == Commands::Authentication) {
        QString name, message;
        in >> name >> message;
        if(name == client.username) {
            QListWidgetItem *item = new QListWidgetItem(message, ui->chatList);
            item->setForeground(Qt::darkGreen);
            ui->chatList->addItem(item);
            ui->saveHistoryAct->setEnabled(true);
            sendToServer(Commands::UpdateDataBase);
        } else {
            QListWidgetItem *item = new QListWidgetItem(message, ui->chatList);
            item->setForeground(Qt::green);
            ui->chatList->addItem(item);
        }

    } else if (static_cast<int>(command) == Commands::Exit) {
        QString name, message;
        in >> name >> message;
        if (name == "Server"){
            QListWidgetItem *item = new QListWidgetItem(message, ui->chatList);
            item->setForeground(Qt::darkRed);
            ui->chatList->addItem(item);
        } else if(name != client.username){
            QListWidgetItem *item = new QListWidgetItem(message, ui->chatList);
            item->setForeground(Qt::darkRed);
            ui->chatList->addItem(item);
            for(int i = 0; i < includedClients.size(); i++){
                if(name == ui->clientList->item(i)->text()){
                    includedClients.removeAt(i);
                    QListWidgetItem *it = ui->clientList->takeItem(i);
                    delete it;
                    break;
                }
            }
        }

    } else if (static_cast<int>(command) == Commands::UpdateDataBase) {
        quint8 countOfClients;
        in >> countOfClients;
        for (int i = 0; i < (int)countOfClients; i++) {
            QString name, status, date, time, path, customStatus;
            in >> name >> status >> date >> time >> path >> customStatus;
            MyClient *ptr = new MyClient("127.0.0.1", 45678, name, status.toInt(), date, time, nullptr, path, customStatus);
            includedClients.push_back(ptr);
            QListWidgetItem *item = new QListWidgetItem(QIcon(path), name, ui->clientList);
            ui->clientList->addItem(item);
        }

    } else if (static_cast<int>(command) == Commands::NewClient) {
        QString name, status, date, time, path, customStatus;
        in >> name >> status >> date >> time >> path >> customStatus;
        MyClient *ptr = new MyClient("127.0.0.1", 45678, name, status.toInt(), date, time, nullptr, path, customStatus);
        includedClients.push_back(ptr);
        QListWidgetItem *item = new QListWidgetItem(QIcon(path), name, ui->clientList);
        ui->clientList->addItem(item);

    } else if (static_cast<int>(command) == Commands::Restart) {
        statusBar()->showMessage("Server reloading...", 2500);
        QListWidgetItem *item = new QListWidgetItem(client.username + " has been disconnected for reload server!", ui->chatList);
        item->setForeground(Qt::darkRed);
        ui->chatList->addItem(item);
        item->setForeground(Qt::darkYellow);
        item->setText("Server has been reloaded!");
        ui->chatList->addItem(item);
        client.socket->disconnectFromHost();

    } else if (static_cast<int>(command) == Commands::DataChanged) {
        statusBar()->showMessage("Sending new data to server...", 2500);
        QString name, message; in >> name >> message;
        if (message == QString::number(Status::Online) ||
        message == QString::number(Status::NotInPlace) ||
        message == QString::number(Status::NotDisturb)) {
            if (name == client.username) {
                QListWidgetItem *item = new QListWidgetItem("Server: Your status has been updated!", ui->chatList);
                item->setForeground(Qt::cyan);
                ui->chatList->addItem(item);
            } else {
                for (MyClient *ptr: includedClients) {
                    if (ptr->username == name) {
                        ptr->status = message.toInt();
                        QListWidgetItem *item = new QListWidgetItem("Server: " + name + " status has been updated!", ui->chatList);
                        item->setForeground(Qt::darkCyan);
                        ui->chatList->addItem(item);
                        break;
                    }
                }
            }
        } else if (message == QString::number(Status::Other)) {
            QString customStatus; in >> customStatus;
            if (name == client.username) {
                QListWidgetItem *item = new QListWidgetItem("Server: Your status has been updated for " + customStatus + "!", ui->chatList);
                item->setForeground(Qt::cyan);
                ui->chatList->addItem(item);
            } else {
                for (MyClient *ptr: includedClients) {
                    if (ptr->username == name) {
                        ptr->status = message.toInt();
                        ptr->statusName = customStatus;
                        QListWidgetItem *item = new QListWidgetItem("Server: " + name + " status has been updated for " + customStatus + "!", ui->chatList);
                        item->setForeground(Qt::darkCyan);
                        ui->chatList->addItem(item);
                        break;
                    }
                }
            }
        } else {
            if (name == client.username) {
                client.username = message;
                QListWidgetItem *item = new QListWidgetItem("Server: Your name has been updated for " + message + "!", ui->chatList);
                item->setForeground(Qt::cyan);
                ui->chatList->addItem(item);
            } else {
                for (MyClient *ptr: includedClients) {
                    if (ptr->username == name) {
                        ptr->username = message;
                        for(int i = 0; i < includedClients.size(); i++){
                            if(name == ui->clientList->item(i)->text()){
                                ui->clientList->item(i)->setText(message);
                                break;
                            }
                        }
                        QListWidgetItem *item = new QListWidgetItem("Server: " + name + " name has been updated for " + message + "!", ui->chatList);
                        item->setForeground(Qt::darkCyan);
                        ui->chatList->addItem(item);
                        break;
                    }
                }
            }
        }
    } else if (static_cast<int>(command) == Commands::Image) {
        QString name, message;
        in >> name >> message;
        QPixmap image;
        image.load(message);
        if(name == client.username) {
            QListWidgetItem *item = new QListWidgetItem(QIcon(image), "You: ", ui->chatList);
            item->setForeground(Qt::black);
            ui->chatList->addItem(item);
        } else {
            if (client.status != Status::NotDisturb){
                QSound::play("/home/kataich75/qtprojects/TECH/TeddyClient/other/newmessage.wav");
            }
            QListWidgetItem *item = new QListWidgetItem(QIcon(image), name + ": ", ui->chatList);
            item->setForeground(Qt::blue);
            ui->chatList->addItem(item);
        }
    } else if (static_cast<int>(command) == Commands::ForceQuit) {
        QListWidgetItem *item = new QListWidgetItem("Validation failed, your name already used!", ui->chatList);
        item->setForeground(Qt::darkYellow);
        ui->chatList->addItem(item);
        isConnected = false;
        client.socket->disconnectFromHost();
    }
}

void ClientWindow::slotEncrypted(){
    isConnected = true;
    client.date = QDate::currentDate().toString();
    client.time = QTime::currentTime().toString();
    sendToServer(Commands::Authentication);
}

//Меню "General"
void ClientWindow::on_connectAct_triggered()
{
    statusBar()->showMessage("Connect to server...", 2500);
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
}

void ClientWindow::on_disconnectAct_triggered()
{
    isConnected = false;
    statusBar()->showMessage("Disconnect from server...", 2500);
    sendToServer(Commands::Exit);
    client.socket->disconnectFromHost();
}
void ClientWindow::slotSocketDisconnected()
{
    ui->connectAct->setEnabled(true);
    ui->disconnectAct->setDisabled(true);
    ui->quitAct->setEnabled(true);
    ui->sendButton->setDisabled(true);
    ui->ipPortAct->setEnabled(true);
    ui->clientList->clear();
    QListWidgetItem *item = new QListWidgetItem("You has been disconnected from server!", ui->chatList);
    item->setForeground(Qt::red);
    ui->chatList->addItem(item);
}

void ClientWindow::on_saveHistoryAct_triggered()
{
    statusBar()->showMessage("Saving history...", 2500);
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
    statusBar()->showMessage("Please, noo...", 2500);
    DialogExit *window = new DialogExit(this);
    window->setWindowTitle("Quiting app");
    window->show();
    connect(window, SIGNAL(closeApplication()), this, SLOT(slotCloseApplication()));
}
void ClientWindow::slotCloseApplication(){
    if (isConnected){
        ui->disconnectAct->triggered();
        client.socket->waitForReadyRead();
    }
    qApp->exit();
}
//Меню "Settings"
void ClientWindow::on_ipPortAct_triggered()
{
    statusBar()->showMessage("Changing your ip & port...", 2500);
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

void ClientWindow::on_actionAvatar_triggered()
{
    client.path = QFileDialog::getOpenFileName(0, QObject::tr("Select photo"),
        "/home/kataich75/qtprojects/TECH/TeddyClient/other/", QObject::tr("Image files (*.png)"));
}

void ClientWindow::on_nameAct_triggered()
{
    statusBar()->showMessage("Changing your name...", 2500);
    DialogUserName *window = new DialogUserName(this, client.username);
    window->setWindowTitle("Edit Your Name");
    window->show();
    connect(window, SIGNAL(dialogUserNameParams(QString)), this, SLOT(slotDialogUserNameParams(QString)));
}
void ClientWindow::slotDialogUserNameParams(QString username)
{
    setWindowTitle(username);
    if (isConnected){
        sendToServer(Commands::DataChanged, username);
    }
    else client.username = username;
}

void ClientWindow::on_actionOnline_triggered()
{
    statusBar()->showMessage("Now you are Online!", 2500);
    if (ui->actionOnline->isChecked()) {
        ui->actionNotInPlace->setChecked(false);
        ui->actionDoNotDisturb->setChecked(false);
        ui->actionOther->setChecked(false);
        client.status = Status::Online;
        ui->labelStatus->setText("Online");
        if (isConnected)
            sendToServer(Commands::DataChanged, QString::number(Status::Online));
    } else ui->actionOnline->setChecked(true);
}
void ClientWindow::on_actionNotInPlace_triggered()
{
    statusBar()->showMessage("Now you are Not In Place!", 2500);
    if (ui->actionNotInPlace->isChecked()) {
        ui->actionOnline->setChecked(false);
        ui->actionDoNotDisturb->setChecked(false);
        ui->actionOther->setChecked(false);
        client.status = Status::NotInPlace;
        ui->labelStatus->setText("Not In Place");
        if (isConnected)
            sendToServer(Commands::DataChanged, QString::number(Status::NotInPlace));
    } else ui->actionNotInPlace->setChecked(true);
}
void ClientWindow::on_actionDoNotDisturb_triggered()
{
    statusBar()->showMessage("Now you are Not Disturb!", 2500);
    if (ui->actionDoNotDisturb->isChecked()) {
        ui->actionOnline->setChecked(false);
        ui->actionNotInPlace->setChecked(false);
        ui->actionOther->setChecked(false);
        client.status = Status::NotDisturb;
        ui->labelStatus->setText("Do Not Disturb");
        if (isConnected)
            sendToServer(Commands::DataChanged, QString::number(Status::NotDisturb));
    } else ui->actionDoNotDisturb->setChecked(true);
}

void ClientWindow::on_actionOther_triggered()
{
    statusBar()->showMessage("Creating your status...", 2500);
    DialogOtherStatus *window = new DialogOtherStatus(this, client.statusName);
    window->setWindowTitle("Create your status");
    window->show();
    connect(window, SIGNAL(dialogOtherStatusParams(QString)), this, SLOT(slotDialogOtherStatusParams(QString)));
}
void ClientWindow::slotDialogOtherStatusParams(QString customStatus){
    statusBar()->showMessage("Now you are " + customStatus + "!", 2500);
    ui->actionOnline->setChecked(false);
    ui->actionNotInPlace->setChecked(false);
    ui->actionDoNotDisturb->setChecked(false);
    ui->actionOther->setChecked(true);
    client.status = Status::Other;
    client.statusName = customStatus;
    ui->labelStatus->setText(customStatus);
    ui->actionOther->setText(customStatus);
    if (isConnected)
        sendToServer(Commands::DataChanged, customStatus, Status::Other);
}
//Меню "Help"
void ClientWindow::on_appAct_triggered()
{
    statusBar()->showMessage("Seems like a really good man...", 2500);
    DialogAboutAutor *window = new DialogAboutAutor(this);
    window->setWindowTitle("About Application");
    window->show();
}

//UI Buttons
void ClientWindow::on_profileButton_clicked()
{
    statusBar()->showMessage("Showing your profile...", 2500);
    DialogAboutClient *window = new DialogAboutClient(this, client);
    window->show();
}

void ClientWindow::on_sendButton_clicked()
{
    statusBar()->showMessage("Sending message...", 2500);
    sendToServer(Commands::SendMessage, ui->messageField->text());
    ui->messageField->clear();
}
void ClientWindow::on_messageField_returnPressed()
{
    statusBar()->showMessage("Sending message...", 2500);
    sendToServer(Commands::SendMessage, ui->messageField->text());
    ui->messageField->clear();
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

void ClientWindow::showContextMenuOnMessageField(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    QAction *clearAct = new QAction("Clear");
    connect(clearAct, &QAction::triggered, [this]() {
            ui->chatList->clear();
        });
    menu->addAction(clearAct);
    menu->popup(ui->chatList->viewport()->mapToGlobal(pos));
}

void ClientWindow::showContextMenuOnSendButton(QPoint pos)
{
    QMenu *btnMenu = new QMenu;
    QAction *sendPic = new QAction("Send Image");
    connect(sendPic, SIGNAL(triggered()), this, SLOT(slotSendPicture()));

    btnMenu->addAction(sendPic);
    btnMenu->popup(ui->sendButton->mapToGlobal(pos));
}
void ClientWindow::slotSendPicture(){
    QString path = QFileDialog::getOpenFileName(0, QObject::tr("Select sending image"),
                   "/home/kataich75/qtprojects/TECH/TeddyClient/other/", QObject::tr("Image files (*.png)"));
    sendToServer(Commands::Image, path);
}
