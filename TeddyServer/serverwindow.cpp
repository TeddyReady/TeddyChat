#include "serverwindow.h"
#include "ui_serverwindow.h"

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
    server = new MyServer;
    connect(server, SIGNAL(serverStarted(bool)), this, SLOT(slotServerStatus(bool)));
    connect(server, SIGNAL(newConnection(QString)), this, SLOT(slotNewConnection(QString)));
    connect(server, SIGNAL(clientDisconnected(QString)), this, SLOT(slotClientDisconnected(QString)));
    server->deployServer();

    ui->IPLabel->setText(server->serverAddress().toString());
    ui->portLabel->setText(QString::number(server->serverPort()));
}

ServerWindow::~ServerWindow() {
    delete ui;
}

void ServerWindow::slotServerStatus(bool online){
    if (online) {
        ui->chatField->addItem(QTime::currentTime().toString() + " Server has been deployed!");
    } else {
        ui->chatField->addItem(QTime::currentTime().toString() + " Fatal Error! Server cannot be deployed");
    }
}

void ServerWindow::slotNewConnection(QString username) {
    ui->chatField->addItem(QTime::currentTime().toString() + " " + username + " has been connected");
    ui->cntUsers->setText(server->getCountOfClients());
    server->sendToClient("Server: Hello, " + username + ", have a nice day :)");
}

void ServerWindow::slotClientDisconnected(QString username) {
    ui->chatField->addItem(QTime::currentTime().toString() + " " + username + " has been disconnected");
    ui->cntUsers->setText(server->getCountOfClients());
    server->sendToClient("Server: " + username + " left current session.");
}
