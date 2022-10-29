#include "clientwindow.h"
#include "ui_clientwindow.h"

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientWindow)
{
    ui->setupUi(this);
    ui->disconnectAct->setDisabled(true);
    ui->sendButton->setDisabled(true);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &ClientWindow::slotReadyRead);
    connect(socket, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
}

ClientWindow::~ClientWindow() {
    delete ui;
}

void ClientWindow::sendToServer(QString str){
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << str;
    socket->write(data);
}

void ClientWindow::slotReadyRead(){
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_12);
    if (in.status() == QDataStream::Ok){
        QString str;
        in >> str;
        ui->incomingField->setTextColor(Qt::blue);
        ui->incomingField->append(str);
    } else {
        ui->incomingField->append("Read Error!");
    }
}

void ClientWindow::on_connectAct_triggered() {
    socket->connectToHost(ip, port);
}
void ClientWindow::on_disconnectAct_triggered() {
    socket->disconnectFromHost();
}
void ClientWindow::on_saveHistoryAct_triggered() {

}
void ClientWindow::on_quitAct_triggered() {
    qApp->exit();
}

void ClientWindow::on_sendButton_clicked()
{
    sendToServer(ui->messageField->text());
}
void ClientWindow::on_messageField_returnPressed()
{
    sendToServer(ui->messageField->text());
}

void ClientWindow::slotSocketConnected(){
    ui->connectAct->setDisabled(true);
    ui->disconnectAct->setEnabled(true);
    ui->sendButton->setEnabled(true);
}
void ClientWindow::slotSocketDisconnected(){
    ui->connectAct->setEnabled(true);
    ui->disconnectAct->setDisabled(true);
    ui->sendButton->setDisabled(true);
    ui->incomingField->clear();
}

void ClientWindow::on_ipPortAct_triggered() {
    DialogIPPort *window = new DialogIPPort(this, ip, port);
    window->setWindowTitle("Edit Your Ip and Port");
    window->show();
    connect(window, SIGNAL(dialogIPPortParams(QString,int)), this, SLOT(slotDialogIPPortParams(QString,int)));
}
void ClientWindow::slotDialogIPPortParams(QString ip, int port){
    this->ip = ip;
    this->port = port;
}

void ClientWindow::on_nameAct_triggered() {
    DialogUserName *window = new DialogUserName(this, username);
    window->setWindowTitle("Edit Your Name");
    window->show();
    connect(window, SIGNAL(dialogUserNameParams(QString)), this, SLOT(slotDialogUserNameParams(QString)));
}
void ClientWindow::slotDialogUserNameParams(QString name){
    username = name;
}

void ClientWindow::on_statusAct_triggered()
{
    DialogStatus *window = new DialogStatus(this, status);
    window->setWindowTitle("Edit Your Status");
    window->show();
    connect(window, SIGNAL(dialogStatusParams(int)), this, SLOT(slotDialogStatusParams(int)));
}
void ClientWindow::slotDialogStatusParams(int status){
    this->status = status;
}
