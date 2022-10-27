#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_connectButton_clicked()
{
    socket->connectToHost("127.0.0.1", 2075);
}

void MainWindow::slotReadyRead(){
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_12);
    if (in.status() == QDataStream::Ok){
        QString str;
        in >> str;
        ui->incomingField->append(str);
    } else {
        ui->incomingField->append("Read Error!");
    }
}

void MainWindow::sendToServer(QString str){
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << str;
    socket->write(data);
}
void MainWindow::on_sendButton_clicked()
{
    sendToServer(ui->messageField->text());
}

void MainWindow::on_messageField_returnPressed()
{
    sendToServer(ui->messageField->text());
}
