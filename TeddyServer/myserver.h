#pragma once
#include <QSslConfiguration>
#include <QDataStream>
#include <QTcpServer>
#include <QSslSocket>
#include <QVector>
#include <QString>
#include <QSslKey>
#include <QTimer>

#include "keygenerator.h"
#include "../myclient.h"
class MyClient;

class MyServer: public QTcpServer {
    Q_OBJECT 
public:
    QString ip;
    int port;
    QVector<MyClient *> clients;
    QSslSocket *socket;
    quint16 dataSize = 0;

    explicit MyServer();
    void deployServer();

    void sendToClient(int command, QString receiver = "", QString message = "", int option = 0);
public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();
signals:
    void serverStarted(bool);
    void newConnection(MyClient *);
    void clientDisconnected(MyClient *);
    void reNameOnUI(QString, QString);
};
