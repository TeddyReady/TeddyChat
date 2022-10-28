#include "clientwindow.h"
#include "ui_clientwindow.h"

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &ClientWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

ClientWindow::~ClientWindow() {
    //socket->disconnectFromHost();
    delete ui;
}

void ClientWindow::on_connectButton_clicked() {
    socket->connectToHost("127.0.0.1", 2075);
    ui->connectButton->setDisabled(true);
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

void ClientWindow::sendToServer(QString str){
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << str;
    socket->write(data);
}
void ClientWindow::on_sendButton_clicked()
{
    sendToServer(ui->messageField->text());
}

void ClientWindow::on_messageField_returnPressed()
{
    sendToServer(ui->messageField->text());
}
