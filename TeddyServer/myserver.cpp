#include "myserver.h"

MyServer::MyServer() {}

void MyServer::deployServer(){
    if (this->listen(QHostAddress::Any, 45678)) {
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
}

void MyServer::slotClientDisconnected(){
    QSslSocket *socket = (QSslSocket*)sender();
    for(MyClient client: clients){
        if(socket->socketDescriptor() == client.socket->socketDescriptor()){
            emit clientDisconnected(client);
            clients.removeOne(client);
            break;
        }
    }
}

void MyServer::slotReadyRead(){
    QSslSocket *socket = (QSslSocket*)sender();
    QDataStream in(socket);
    if (dataSize == 0) {
        if (socket->bytesAvailable() < (int)sizeof(quint16)){
            return;
        }
        in >> dataSize;
        qDebug() << "dataSize now " << dataSize;
    }
    if (socket->bytesAvailable() < dataSize){
        return;
    } else {
        dataSize = 0;
    }
    QString key, message, name, status;
    in.setVersion(QDataStream::Qt_5_12);
    if (in.status() == QDataStream::Ok){
        in >> key;
        if(key == "MESSAGE"){
            in >> name >> message;
            sendToClient(name + ": " + message);
        }
        if(key == "DATA"){
            in >> name >> status;
            MyClient client("127.0.0.1", 45678, name, status.toInt(), socket);
            clients.push_back(client);
            emit newConnection(client);
        }
    } else {
       qDebug() << "Read error!";
    }
}
//Для отправки сообщений
void MyServer::sendToClient(QString str){
    QByteArray data; data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << (quint16)0 << (QString)"MESSAGE" << str;
    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    for (MyClient client: clients){
        client.socket->write(data);
    }
}
//Для отправки служебных данных
void MyServer::updateDataInfo(QString mode, MyClient newClient, QString serverMsg){
    if (mode == "ADD"){
        QByteArray mainData; mainData.clear();
        QDataStream generalOut(&mainData, QIODevice::WriteOnly);
        generalOut.setVersion(QDataStream::Qt_5_12);
        generalOut << (quint16)0 << (QString)"DATA" << serverMsg << (QString)(clients.size() - 1);

        for(MyClient curClient: clients){
            if(curClient != newClient){
                QByteArray data; data.clear();
                QDataStream localOut(&data, QIODevice::WriteOnly);
                localOut.setVersion(QDataStream::Qt_5_12);
                localOut << (quint16)0 << (QString)"NEWCLIENT" << serverMsg << newClient.username << (QString)newClient.status;
                generalOut << curClient.username << (QString)curClient.status;
                localOut.device()->seek(0);
                localOut << quint16(data.size() - sizeof(quint16));
                curClient.socket->write(data);
            }
        }
        generalOut.device()->seek(0);
        generalOut << quint16(mainData.size() - sizeof(quint16));
        qDebug() << quint16(mainData.size() - sizeof(quint16));
        newClient.socket->write(mainData);
    } /*else {
        QByteArray mainData; mainData.clear();
        QDataStream generalOut(&mainData, QIODevice::WriteOnly);
        generalOut.setVersion(QDataStream::Qt_5_12);
        generalOut << (QString)"DELETE" << serverMsg << (QString)(clients.size() - 1);

        for(MyClient curClient: clients){
            if(curClient != newClient){
                QByteArray data; data.clear();
                QDataStream localOut(&data, QIODevice::WriteOnly);
                localOut.setVersion(QDataStream::Qt_5_12);
                localOut << (QString)"NEWCLIENT" << newClient.username << (QString)newClient.status;
                curClient.socket->write(data);
                generalOut << curClient.username << (QString)curClient.status;
            }
        }
        newClient.socket->write(mainData);
    }*/
}
