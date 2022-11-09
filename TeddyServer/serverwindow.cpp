#include "serverwindow.h"
#include "ui_serverwindow.h"

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
    server = new MyServer;
    connect(server, SIGNAL(serverStarted(bool)), this, SLOT(slotServerStatus(bool)));
    connect(server, SIGNAL(newConnection(MyClient *)), this, SLOT(slotNewConnection(MyClient *)));
    connect(server, SIGNAL(clientDisconnected(MyClient *)), this, SLOT(slotClientDisconnected(MyClient *)));
    server->deployServer();

    ui->IPLabel->setText(server->serverAddress().toString());
    ui->portLabel->setText(QString::number(server->serverPort()));
}

ServerWindow::~ServerWindow() {
    delete ui;
}

void ServerWindow::slotServerStatus(bool online){
    if (online) {
        ui->chatField->setTextColor(Qt::green);
        ui->chatField->append(QTime::currentTime().toString() + " Server has been deployed!");
    } else {
        ui->chatField->setTextColor(Qt::red);
        ui->chatField->append(QTime::currentTime().toString() + " Fatal Error! Server cannot be deployed");
    }
}

void ServerWindow::slotNewConnection(MyClient *client) {
    ui->chatField->setTextColor(Qt::darkGreen);
    ui->chatField->append(client->time + " " + client->username + " has been connected");
    ui->cntUsers->setText(QString::number(server->clients.size()));
    ui->clientList->addItem(client->username);
}

void ServerWindow::slotClientDisconnected(MyClient *client) {
    ui->chatField->setTextColor(Qt::darkRed);
    ui->chatField->append(client->time + " " + client->username + " has been disconnected");
    ui->cntUsers->setText(QString::number(server->clients.size() - 1));
    //Removing client from UI
    for(int i = 0; i < server->clients.size(); i++){
        if(client->username == ui->clientList->item(i)->text()){
            QListWidgetItem *it = ui->clientList->takeItem(i);
            delete it;
            break;
        }
    }
}
