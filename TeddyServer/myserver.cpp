#include "myserver.h"

MyServer::MyServer() {}

void MyServer::deployServer(){
    if (this->listen(QHostAddress::LocalHost, 45678)) {
        emit serverStarted(true);
    } else {
        emit serverStarted(false);
    }
}

void MyServer::incomingConnection(qintptr socketDescriptor){
    QSslSocket *socket = new QSslSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(slotClientDisconnected()));
    clients.push_back(socket);
    emit newConnection("");
}

void MyServer::slotClientDisconnected(){
    QSslSocket *socket = (QSslSocket*)sender();
    clients.removeOne(socket);
    emit clientDisconnected("");
}

void MyServer::slotReadyRead(){
    QSslSocket *socket = (QSslSocket*)sender();
    QString username, message;
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_12);
    if (in.status() == QDataStream::Ok){
        in >> username >> message;
        sendToClient(username + ": " + message);
    } else {
       //qDebug() << "Read error!";
    }
}

void MyServer::sendToClient(QString str){
    QByteArray data;
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << str;
    for (int i = 0; i < clients.size(); i++){
        clients[i]->write(data);
    }
}
