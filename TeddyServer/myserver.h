#pragma once
#include <QTcpServer>
#include <QSslSocket>
#include <QVector>
#include <QDebug>
#include <QDataStream>
#include <QString>
#include "/home/kataich75/qtprojects/TECH/TeddyClient/clientwindow.h"

class MyServer: public QTcpServer {
    Q_OBJECT
private:
    QVector<QSslSocket *> sockets;
    QByteArray data;
    void sendToClient(QString str);
public:
    explicit MyServer();
    QSslSocket *socket;

    QString getCountOfClients() const {return QString::number(sockets.size());}
public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();

signals:
    void newConnection(QString);
    void clientDisconnected(QString);
};
