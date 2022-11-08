#include "myserver.h"

MyServer::MyServer() {
    ip = "127.0.0.1";
    port = 45678;
}
void MyServer::deployServer(){
    if (this->listen(QHostAddress::LocalHost, port)) {
        emit serverStarted(true);
    } else {
        emit serverStarted(false);
    }
}

void MyServer::incomingConnection(qintptr socketDescriptor){
    socket = new QSslSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
}

void MyServer::sendToClient(int command, QString receiver, QString message){
    QByteArray data; data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << (quint16)0 << (quint8)command;
    if(command == Commands::SendMessage || command == Commands::Authentication || command == Commands::Exit) {
        out << receiver << message;
        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));
        for(int i = 0; i < clients.size(); i++){
            clients[i]->socket->write(data);
        }
    } else if (command == Commands::UpdateDataBase) {
        out << (quint8)(clients.size() - 1);
        qDebug() << (quint8)(clients.size() - 1);
        for (int i = 0; i < clients.size() - 1; i++){
            qDebug() << "Captured!";
            out << clients[i]->username
                << QString::number(clients[i]->status);
        }
        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));
        clients[clients.size() - 1]->socket->write(data);
    } else if (command == Commands::NewClient) {
        out << clients[clients.size() - 1]->username
            << QString::number(clients[clients.size() - 1]->status);
        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));
        clients[receiver.toInt()]->socket->write(data);
    }
}
void MyServer::slotReadyRead(){
    socket = (QSslSocket*)sender();
    QDataStream in(socket);
    //PACKET_DEFENCER
    if (dataSize == 0) {
        if (socket->bytesAvailable() < (int)sizeof(quint16)) return;
        in >> dataSize;
    }
    if (socket->bytesAvailable() < dataSize) return;
    else dataSize = 0;
    //END
    quint8 command;
    in >> command;
    if((int)command == Commands::SendMessage) {
        QString name, message;
        in >> name >> message;
        sendToClient(Commands::SendMessage, name ,": " + message);
    } else if((int)command == Commands::Authentication){
        QString ip, port, name, status;
        in >> ip >> port >> name >> status;
            MyClient *ptr = new MyClient(ip, port.toInt(), name, status.toInt(), socket);
            clients.push_back(ptr);
            sendToClient(Commands::Authentication, ptr->username, "Server: " + ptr->username + " join chat channel!");
            emit newConnection(ptr);
    } else if ((int)command == Commands::Exit){
        QString name;
        in >> name;
        for(int i = 0; i < clients.size(); i++){
            if(name == clients[i]->username){
                sendToClient(Commands::Exit, name, "Server: " + name + " leaved current session!");
                emit clientDisconnected(clients[i]);
                clients.removeAt(i);
                break;
            }
        }
    } else if ((int)command == Commands::UpdateDataBase) {
        sendToClient(Commands::UpdateDataBase);
        for (int i = 0; i < clients.size() - 1; i++)
            sendToClient(Commands::NewClient, QString::number(i));
    }
}
