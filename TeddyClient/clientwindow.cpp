#include "clientwindow.h"
#include "ui_clientwindow.h"

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientWindow)
{
    settings = new QSettings("/home/kataich75/qtprojects/TECH/TeddyClient/settings.ini", QSettings::IniFormat, this);
    uploadSettings();

    ui->setupUi(this);
    ui->disconnectAct->setDisabled(true);
    ui->sendButton->setDisabled(true);
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

    QSslSocket *socket = new QSslSocket(this);
    client.socket = socket;
    connect(client.socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(client.socket, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(client.socket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));

}

ClientWindow::~ClientWindow()
{
    saveSettings();
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

void ClientWindow::sendMessageToServer(QString str)
{
    QByteArray data; data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << (quint16)0 << (QString)"MESSAGE" << client.username << str;
    out.device()->seek(0);
    out << (quint16)(data.size() - sizeof(quint16));
    client.socket->write(data);
}

void ClientWindow::sendDataToServer()
{
    QByteArray data; data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << (quint16)0 << (QString)"DATA" << client.username << (QString)client.status;
    out.device()->seek(0);
    out << (quint16)(data.size() - sizeof(quint16));
    client.socket->write(data);
}

void ClientWindow::slotReadyRead()
{
    QDataStream in(client.socket);
    if (dataSize == 0) {
        if (client.socket->bytesAvailable() < (int)sizeof(quint16)){
            return;
        }
        in >> dataSize;
        qDebug() << "dataSize now " << dataSize;
    }
    if (client.socket->bytesAvailable() < dataSize){
        return;
    } else {
        dataSize = 0;
    }
    QString keyword;
    in >> keyword;
    qDebug() << keyword;

    if(keyword == "MESSAGE") {
        if (client.status != Status::NotDisturb){
            QSound::play("/home/kataich75/qtprojects/TECH/TeddyClient/newmessage.wav");
        }
        QString str;
        in >> str;
        qDebug() << str;
        ui->incomingField->setTextColor(Qt::blue);
        ui->incomingField->append(str);

    } else if(keyword == "NEWCLIENT") {
        QString str, name, status;
        in >> str >> name >> status;
        qDebug() << str << name << status;
        ui->incomingField->setTextColor(Qt::darkGreen);
        ui->incomingField->append(str);
        MyClient newClient("127.0.0.1", 45678, name, status.toInt());
        includedClients.push_back(newClient);
        ui->clientList->addItem(newClient.username);

    } else if(keyword == "DATA") {
        QString str, cnt;
        in >> str >> cnt;
        qDebug() << str << cnt;
        ui->incomingField->setTextColor(Qt::green);
        ui->incomingField->append(str);
        for (int i = 0; i < cnt.toInt(); i++){
            qDebug() << "YEEES";
            QString name, status;
            in >> name >> status;
            qDebug() << name << status;
            MyClient newClient("127.0.0.1", 45678, name, status.toInt());
            includedClients.push_back(newClient);
            ui->clientList->addItem(newClient.username);
        }
    }
}

//Menu "File"
void ClientWindow::on_connectAct_triggered()
{
    client.socket->connectToHost(client.ip, client.port);
    sendDataToServer();
    dataSize = 0;
}
void ClientWindow::slotSocketConnected()
{
    ui->connectAct->setDisabled(true);
    ui->disconnectAct->setEnabled(true);
    ui->sendButton->setEnabled(true);
    ui->ipPortAct->setDisabled(true);
}

void ClientWindow::on_disconnectAct_triggered()
{
    client.socket->disconnectFromHost();
}
void ClientWindow::slotSocketDisconnected()
{
    ui->connectAct->setEnabled(true);
    ui->disconnectAct->setDisabled(true);
    ui->sendButton->setDisabled(true);
    ui->incomingField->clear();
    ui->ipPortAct->setEnabled(true);
}

void ClientWindow::on_saveHistoryAct_triggered()
{

}
void ClientWindow::on_quitAct_triggered()
{
    qApp->exit();
}

//Menu "Settings"
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
//Menu "Help"
void ClientWindow::on_appAct_triggered()
{
    DialogAboutAutor *window = new DialogAboutAutor(this);
    window->setWindowTitle("About Application");
    window->show();
}

//UI Fields
void ClientWindow::on_sendButton_clicked()
{
    sendMessageToServer(ui->messageField->text());
    ui->messageField->clear();
}
void ClientWindow::on_messageField_returnPressed()
{
    sendMessageToServer(ui->messageField->text());
    ui->messageField->clear();
}
