#pragma once
#include <QSslConfiguration>
#include <QDataStream>
#include <QFileDialog>
#include <QTcpServer>
#include <QSslSocket>
#include <QVector>
#include <QString>
#include <QSslKey>
#include <QTimer>
#include <QFile>

#include "keygenerator.h"
#include "../myclient.h"
#include "constants.h"
class MyClient;

class MyServer: public QTcpServer {
    Q_OBJECT 
public:
    QString ip;
    int port;
    QVector<MyClient *> clients;
    QVector<QString> sendList;
    QSslSocket *socket;
    quint64 dataSize = 0;

    explicit MyServer();
    ~MyServer();

    void deployServer();
    void sendToClient(int command, QString receiver = "", QString message = "", int option = 0);
public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();
signals:
    void serverStarted(bool);
    void newConnection(MyClient *);
    void failedValidation();
    void clientDisconnected(MyClient *);
    void reNameOnUI(QString, QString);
    void updateProgressBar(int);
};
