#include "myserver.h"

MyServer::MyServer()
{
    if (this->listen(QHostAddress::Any, 2075)) {
        qDebug("Server started...");
    } else {
        qDebug("Fatal error! Server cannot start");
    }
}

void MyServer::incomingConnection(qintptr socketDescriptor){
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &MyServer::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    sockets.push_back(socket);
    qDebug("Client has been connected!", socketDescriptor);
}

void MyServer::slotReadyRead(){
   socket = (QTcpSocket*)sender();
   QDataStream in(socket);
   in.setVersion(QDataStream::Qt_5_12);
   if (in.status() == QDataStream::Ok){
       qDebug("Read...");
       QString str;
       in >> str;
       //qDebug(str);
       sendToClient(str);
   } else {
       qDebug("Read error!");
   }
}

void MyServer::sendToClient(QString str){
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << str;
    for (size_t i = 0; i < sockets.size(); i++){
        sockets[i]->write(data);
    }
}
