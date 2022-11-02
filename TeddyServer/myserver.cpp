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
    qDebug() << socketDescriptor;
    connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(slotClientDisconnected()));
}

void MyServer::slotClientDisconnected(){
    QSslSocket *socket = (QSslSocket*)sender();
    for(MyClient client: clients){
        qDebug() << socket->socketDescriptor() << " vs " << client.socket->socketDescriptor();
        if(socket->socketDescriptor() == client.socket->socketDescriptor()){
            clients.removeOne(client);
            emit clientDisconnected(client.username);
            break;
        }
    }
}

void MyServer::slotReadyRead(){
    QString key, message, name, status;
    QSslSocket *socket = (QSslSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_12);
    if (in.status() == QDataStream::Ok){
        in >> key;
        if(key == "MSSG"){
            in >> name >> message;
            sendToClient(name + ": " + message);
        } else {
            in >> name >> status;
            MyClient client("127.0.0.1", 45678, name, status.toInt(), socket);
            clients.push_back(client);
            emit newConnection(client.username);
        }
    } else {
       qDebug() << "Read error!";
    }
}

void MyServer::sendToClient(QString str){
    QByteArray data; data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << str;
    for (int i = 0; i < clients.size(); i++){
        clients[i].socket->write(data);
    }
}
