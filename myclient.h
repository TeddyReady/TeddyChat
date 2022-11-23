#pragma once
#include <QSslSocket>
#include <QDateTime>
#include <QString>
#include <QIcon>

enum Status {Online, NotInPlace, NotDisturb, Other};
enum Commands {
    SendMessage, Authentication, Exit, 
        UpdateDataBase, NewClient,
    DataChanged, Restart, Image,
        ForceQuit, PathChanged,
    ColorChanged, ForbiddenName
};

class MyClient {
public:
    QString ip;
    int port;
    QString username;
    int status;
    QString date, time, path, statusName, colorName;
    QSslSocket *socket;

    MyClient(QString ip = "127.0.0.1", int port = 45678, QString username = "Unknown User",
             int status = Status::Online, QString date = "no info",
             QString time = "no info", QSslSocket *socket = nullptr, QString path = ":/new/prefix1/other/client.png",
             QString statusName = "Working on lab...", QString colorName = "white")
        : ip(ip), port(port), username(username), status(status), date(date), time(time),
          path(path), statusName(statusName), colorName(colorName) {
        this->socket = socket;
    }
    ~MyClient() {}
};

