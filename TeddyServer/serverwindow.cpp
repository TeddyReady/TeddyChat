#include "serverwindow.h"
#include "ui_serverwindow.h"

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
    server = new MyServer;
    ui->IPLabel->setText(server->serverAddress().toString());
    ui->portLabel->setText(QString::number(server->serverPort()));
    ui->chatField->addItem(QTime::currentTime().toString()+" server started at "+server->serverAddress().toString()+":"+QString::number(server->serverPort()));
    connect(server, SIGNAL(newConnection(QString)), this, SLOT(slotNewConnection(QString)));
    connect(server, SIGNAL(clientDisconnected(QString)), this, SLOT(slotClientDisconnected(QString)));
}

ServerWindow::~ServerWindow() {
    delete ui;
}

void ServerWindow::slotNewConnection(QString message) {
    ui->chatField->addItem(QTime::currentTime().toString()+ " " + message);
    ui->cntUsers->setText(server->getCountOfClients());
}

void ServerWindow::slotClientDisconnected(QString message) {
    ui->chatField->addItem(QTime::currentTime().toString()+ " " +message);
    ui->cntUsers->setText(server->getCountOfClients());
}
