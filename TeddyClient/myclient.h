#pragma once
#include <QString>
#include "/home/kataich75/qtprojects/TECH/TeddyServer/myserver.h"

enum Status {Online, NotInPlace, NotDisturb};

class MyClient {
public:
    QString ip;
    int port;
    QString username;
    int status;
    QSslSocket *socket;

    MyClient(QString ip = "127.0.0.1", int port = 45678, QString username = "Unknown User",
                      int status = Status::Online, QSslSocket *socket = nullptr)
        : ip(ip), port(port), username(username), status(status) {
        this->socket = socket;
    }

    ~MyClient()
    {
       ip.clear();
       username.clear();
    }

    bool operator==(const MyClient &client)
    {
        if(socket->socketDescriptor() == client.socket->socketDescriptor()){
            return true;
        } else {
            return false;
        }
    }
    bool operator!=(const MyClient &client)
    {
        if(*this == client){
           return false;
        } else {
            return true;
        }
    }
};

