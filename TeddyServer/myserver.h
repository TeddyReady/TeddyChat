#pragma once
#include <QTcpServer>
#include <QSslSocket>
#include <QVector>
#include <QDebug>
#include <QDataStream>
#include <QString>

class MyServer: public QTcpServer {
    Q_OBJECT
private:
    QVector<QSslSocket *> clients;
    void sendToClient(QString str);
public:
    explicit MyServer();
    void deployServer();

    QVector<QSslSocket *> getCurrentClients() const {return clients;}
    QString getCountOfClients() const {return QString::number(clients.size());}
public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotClientDisconnected();
    void slotReadyRead();

signals:
    void serverStarted(bool);
    void newConnection(QString);
    void clientDisconnected(QString);
};
