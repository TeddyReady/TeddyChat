#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QDebug>
#include <QDataStream>
#include <QString>

class MyServer: public QTcpServer {
    Q_OBJECT
private:
    QVector<QTcpSocket *> sockets;
    QByteArray data;
    void sendToClient(QString str);
public:
    explicit MyServer();
    QTcpSocket *socket;

    QString getCountOfClients() const {return QString::number(sockets.size());}
public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();
};
