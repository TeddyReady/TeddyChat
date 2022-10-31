#include "myserver.h"

MyServer::MyServer()
{
    if (this->listen(QHostAddress::LocalHost, 2075)) {
        qDebug() << "Server started...";
    } else {
        qDebug() << "Fatal error! Server cannot start";
    }
}

void MyServer::incomingConnection(qintptr socketDescriptor){
    socket = new QSslSocket;
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QSslSocket::readyRead, this, &MyServer::slotReadyRead);
    connect(socket, &QSslSocket::disconnected, socket, [this]()
    {
        sockets.removeOne(socket);
        emit clientDisconnected((QString)"Client " + (QString)" has been removed!");
    });
    sockets.push_back(socket);
    emit newConnection((QString)"Client " + (QString)" has been connected!");
}

void MyServer::slotReadyRead(){
   socket = (QSslSocket*)sender();
   QDataStream in(socket);
   in.setVersion(QDataStream::Qt_5_12);
   if (in.status() == QDataStream::Ok){
       //qDebug() << "Read...";
       QString str;
       in >> str;
       sendToClient(str);
   } else {
       //qDebug() << "Read error!";
   }
}

void MyServer::sendToClient(QString str){
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << str;
    for (int i = 0; i < sockets.size(); i++){
        sockets[i]->write(data);
    }
}
