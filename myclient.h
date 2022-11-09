#pragma once
#include <QString>
#include <QDateTime>
#include <QSslSocket>

enum Status {Online, NotInPlace, NotDisturb};
enum Commands {
    SendMessage, Authentication, Exit, 
        UpdateDataBase, NewClient
};

class MyClient {
public:
    QString ip;
    int port;
    QString username;
    int status;
    QString date, time;

    QSslSocket *socket;

    MyClient(QString ip = "127.0.0.1", int port = 45678, QString username = "Unknown User",
                      int status = Status::Online, QString date = "no info", QString time = "no info", QSslSocket *socket = nullptr)
        : ip(ip), port(port), username(username), status(status), date(date), time(time) {
        this->socket = socket;
    }

    ~MyClient() {}

    bool operator==(const MyClient &client)
    {
        if(username == client.username){
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

