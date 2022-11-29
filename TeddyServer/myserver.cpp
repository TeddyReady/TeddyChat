#include "myserver.h"

MyServer::MyServer() {}
MyServer::~MyServer() {}

void MyServer::deployServer(){
    if (this->listen(static_cast<QHostAddress>(ip), port)) {
        emit serverStarted(true);
    } else {
        emit serverStarted(false);
    }
}

void MyServer::incomingConnection(qintptr socketDescriptor) {
    MyThread *thread = new MyThread(socketDescriptor, this);

    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(threadNewConnection(MyClient *)), this, SLOT(slotNewConnection(MyClient*)));
    connect(thread, SIGNAL(threadFailedValidation()), this, SLOT(slotFailedValidation()));
    connect(thread, SIGNAL(threadClientDisconnected(MyClient *)), this, SLOT(slotClientDisconnected(MyClient*)));
    connect(thread, SIGNAL(threadReNameOnUI(QString, QString)), this, SLOT(slotReNameOnUI(QString, QString)));
    connect(thread, SIGNAL(threadUpdateProgressBar(int)), this, SLOT(slotUpdateProgressBar(int)));
    connect(thread, SIGNAL(threadAddClient(MyClient *)), this, SLOT(slotAddClient(MyClient *)));
    connect(thread, SIGNAL(threadRemoveClient(MyClient *)), this, SLOT(slotRemoveClient(MyClient *)));
    connect(this, SIGNAL(updateClients(QVector<MyClient *>)), thread, SLOT(slotThreadUpdateClients(QVector<MyClient *>)));
    connect(this, SIGNAL(cmdRestart()), thread, SLOT(slotCmdRestart()));
    connect(this, SIGNAL(cmdExit(QString, QString)), thread, SLOT(slotCmdExit(QString, QString)));

    thread->start();
}

void MyServer::slotNewConnection(MyClient *client){
    emit newConnection(client);
}
void MyServer::slotFailedValidation(){
    emit failedValidation();
}
void MyServer::slotClientDisconnected(MyClient *client){
    emit clientDisconnected(client);
}
void MyServer::slotReNameOnUI(QString name, QString newName){
    emit reNameOnUI(name, newName);
}
void MyServer::slotUpdateProgressBar(int value) {
    emit updateProgressBar(value);
}
void MyServer::slotAddClient(MyClient *client){
    clients.push_back(client);
    emit updateClients(clients);
}
void MyServer::slotRemoveClient(MyClient *client){
    for (int i = 0; i < clients.size(); i++) {
        if (client->username == clients[i]->username) {
            clients.removeAt(i);
            emit updateClients(clients);
            return;
        }
    }
}
void MyServer::slotCmdRestart(){
    emit cmdRestart();
}
void MyServer::slotCmdExit(QString name, QString msg){
    emit cmdExit(name, msg);
}

