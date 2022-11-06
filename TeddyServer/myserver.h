#pragma once
#include <QTcpServer>
#include <QSslSocket>
#include <QVector>
#include <QDebug>
#include <QDataStream>
#include <QString>
#include "/home/kataich75/qtprojects/TECH/TeddyClient/myclient.h"

class MyClient;

class MyServer: public QTcpServer {
    Q_OBJECT
public:
    QVector<MyClient> clients;
    quint16 dataSize = 0;

    explicit MyServer();
    void deployServer();

    void sendToClient(QString str);
    void updateDataInfo(QString mode, MyClient newClient, QString serverMsg);
public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotClientDisconnected();
    void slotReadyRead();

signals:
    void serverStarted(bool);
    void newConnection(MyClient);
    void clientDisconnected(MyClient);
};
