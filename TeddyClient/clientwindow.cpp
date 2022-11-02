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
    ui->labelIP->setText(ip);
    ui->labelPort->setText(QString::number(port));
    switch(status){
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

    socket = new QSslSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(socket, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));

}

ClientWindow::~ClientWindow()
{
    saveSettings();
    delete ui;
}

void ClientWindow::uploadSettings(){
    ip = settings->value("ip", "127.0.0.1").toString();
    port = settings->value("port", "45678").toInt();
    username = settings->value("username", "Unknown User").toString();
    status = settings->value("status", Status::Online).toInt();
    setGeometry(settings->value("geometry", QRect(200, 200, 530, 388)).toRect());
}
void ClientWindow::saveSettings(){
    settings->setValue("ip", ip);
    settings->setValue("port", port);
    settings->setValue("username", username);
    settings->setValue("status", status);
    settings->setValue("geometry", geometry());
}

void ClientWindow::sendToServer(QString str)
{
    QByteArray data; data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << username << str;
    socket->write(data);
}

void ClientWindow::slotReadyRead()
{
    QSound *sound = new QSound("/home/kataich75/qtprojects/TECH/TeddyClient/newmessage.wav");
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_12);
    if (in.status() == QDataStream::Ok){
        if (status != Status::NotDisturb){;
            sound->play();
        }
        QString str;
        in >> str;
        ui->incomingField->setTextColor(Qt::blue);
        ui->incomingField->append(str);
    } else {
        ui->incomingField->append("Read Error!");
    }
}

//Menu "File"
void ClientWindow::on_connectAct_triggered()
{
    socket->connectToHost(ip, port);
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
    socket->disconnectFromHost();
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
    DialogIPPort *window = new DialogIPPort(this, ip, port);
    window->setWindowTitle("Edit Your Ip and Port");
    window->show();
    connect(window, SIGNAL(dialogIPPortParams(QString,int)), this, SLOT(slotDialogIPPortParams(QString,int)));
}
void ClientWindow::slotDialogIPPortParams(QString ip, int port)
{
    this->ip = ip;
    this->port = port;
    ui->labelIP->setText(ip);
    ui->labelPort->setText(QString::number(port));
}

void ClientWindow::on_nameAct_triggered()
{
    DialogUserName *window = new DialogUserName(this, username);
    window->setWindowTitle("Edit Your Name");
    window->show();
    connect(window, SIGNAL(dialogUserNameParams(QString)), this, SLOT(slotDialogUserNameParams(QString)));
}
void ClientWindow::slotDialogUserNameParams(QString username)
{
    this->username = username;
}

void ClientWindow::on_actionOnline_triggered()
{
    if (ui->actionOnline->isChecked()) {
        ui->actionNotInPlace->setChecked(false);
        ui->actionDoNotDisturb->setChecked(false);
        status = Status::Online;
        ui->labelStatus->setText("Online");
    }
}
void ClientWindow::on_actionNotInPlace_triggered()
{
    if (ui->actionNotInPlace->isChecked()) {
        ui->actionOnline->setChecked(false);
        ui->actionDoNotDisturb->setChecked(false);
        status = Status::NotInPlace;
        ui->labelStatus->setText("Not In Place");
    }
}
void ClientWindow::on_actionDoNotDisturb_triggered()
{
    if (ui->actionDoNotDisturb->isChecked()) {
        ui->actionOnline->setChecked(false);
        ui->actionNotInPlace->setChecked(false);
        status = Status::NotDisturb;
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
    sendToServer(ui->messageField->text());
    ui->messageField->clear();
}
void ClientWindow::on_messageField_returnPressed()
{
    sendToServer(ui->messageField->text());
    ui->messageField->clear();
}
