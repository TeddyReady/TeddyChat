#pragma once
#include <QTcpServer>
#include "mythread.h"
class MyClient;

class MyServer: public QTcpServer {
    Q_OBJECT 
public:
    QVector<MyClient *> clients;
    QString ip;
    int port;

    explicit MyServer();
    ~MyServer();

    void deployServer();
signals:
    void serverStarted(bool);
    void newConnection(MyClient *);
    void failedValidation();
    void clientDisconnected(MyClient *);
    void reNameOnUI(QString, QString);
    void updateProgressBar(int);
    void updateClients(QVector<MyClient *>);
    void cmdRestart();
    void cmdExit(QString, QString);
public slots:
    void incomingConnection(qintptr);

    void slotNewConnection(MyClient *);
    void slotFailedValidation();
    void slotClientDisconnected(MyClient *);
    void slotReNameOnUI(QString, QString);
    void slotUpdateProgressBar(int);
    void slotAddClient(MyClient *);
    void slotRemoveClient(MyClient *);

    void slotCmdRestart();
    void slotCmdExit(QString, QString);
};
