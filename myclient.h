#pragma once
#include <QSslSocket>
#include <QDateTime>
#include <QString>
#include <QIcon>

enum Status {Online, NotInPlace, NotDisturb, Other};
enum Commands {
    SendMessage, Authentication, Exit, 
        UpdateDataBase, NewClient,
    DataChanged, Restart
};

class MyClient {
public:
    QString ip;
    int port;
    QString username;
    int status;
    QString date, time;
    QString statusName;

    QSslSocket *socket;
    QIcon *avatar;

    MyClient(QString ip = "127.0.0.1", int port = 45678, QString username = "Unknown User",
             int status = Status::Online, QString date = "no info",
             QString time = "no info", QSslSocket *socket = nullptr)
        : ip(ip), port(port), username(username), status(status), date(date), time(time) {
        this->socket = socket;
        this->avatar = new QIcon(":/new/prefix1/other/client.png");
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

