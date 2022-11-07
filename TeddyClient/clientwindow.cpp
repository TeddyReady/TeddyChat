#include "clientwindow.h"
#include "ui_clientwindow.h"

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientWindow)
{
    settings = new QSettings("../settings.ini", QSettings::IniFormat, this);
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

    client.socket = new QSslSocket(this);
    connect(client.socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(client.socket, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(client.socket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
}

ClientWindow::~ClientWindow()
{
    saveSettings();
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
            << client.username << QString::number(client.status);
    } else if (command == Commands::Exit) {
        out << client.username;
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

    if((int)command == Commands::SendMessage) {
        QString name, message;
        in >> name >> message;
        if(name == client.username) {
            ui->incomingField->setTextColor(Qt::black);
            ui->incomingField->append("You" + message);
        } else {
            if (client.status != Status::NotDisturb){
                QSound::play("/home/kataich75/qtprojects/TECH/TeddyClient/newmessage.wav");
            }
            ui->incomingField->setTextColor(Qt::blue);
            ui->incomingField->append(name + message);
        }

    } else if ((int)command == Commands::Authentication) {
        QString name, message;
        in >> name >> message;
        if(name == client.username) ui->incomingField->setTextColor(Qt::darkGreen);
        else ui->incomingField->setTextColor(Qt::green);
        ui->incomingField->append(message);
    } else if ((int)command == Commands::Exit) {
        QString name, message;
        in >> name >> message;
        if(name != client.username){
            ui->incomingField->setTextColor(Qt::darkRed);
            ui->incomingField->append(message);
        }
    }
}

//Меню "File"
void ClientWindow::on_connectAct_triggered()
{
    client.socket->connectToHost(client.ip, client.port);
    sendToServer(Commands::Authentication);
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
    sendToServer(Commands::Exit);
    client.socket->disconnectFromHost();
}
void ClientWindow::slotSocketDisconnected()
{
    ui->connectAct->setEnabled(true);
    ui->disconnectAct->setDisabled(true);
    ui->sendButton->setDisabled(true);
    ui->incomingField->clear();
    ui->ipPortAct->setEnabled(true);
    ui->clientList->clear();
}

void ClientWindow::on_saveHistoryAct_triggered(){}
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

//Отправка СМС
void ClientWindow::on_sendButton_clicked()
{
    sendToServer(Commands::SendMessage, ui->messageField->text());
    ui->messageField->clear();
}
void ClientWindow::on_messageField_returnPressed()
{
    sendToServer(Commands::SendMessage, ui->messageField->text());
    ui->messageField->clear();
}
