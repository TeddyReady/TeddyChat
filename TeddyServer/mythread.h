#ifndef MYTHREAD_H
#define MYTHREAD_H
#include <QSslConfiguration>
#include <QDataStream>
#include <QFileDialog>
#include <QSslSocket>
#include <QVector>
#include <QString>
#include <QSslKey>
#include <QThread>
#include <QTimer>
#include <QFile>

#include "keygenerator.h"
#include "../myclient.h"
#include "constants.h"

class MyThread : public QThread {
    Q_OBJECT
public:
    QVector<MyClient *> clients;
    QVector<QString> sendList;
    QSslSocket *socket;
    qintptr socketDescriptor;
    quint64 dataSize = 0;

    explicit MyThread(qintptr ID, QObject *parent = 0);
    void run();
    void sendToClient(int command, QString receiver = "", QString message = "", int option = 0);
signals:
    void error(QTcpSocket::SocketError socketerror);
    void threadNewConnection(MyClient *);
    void threadFailedValidation();
    void threadClientDisconnected(MyClient *);
    void threadReNameOnUI(QString, QString);
    void threadUpdateProgressBar(int);
    void threadAddClient(MyClient *);
    void threadRemoveClient(MyClient *);
public slots:
    void slotReadyRead();
    void slotThreadUpdateClients(QVector<MyClient *>);
    void slotCmdRestart();
    void slotCmdExit(QString, QString);
};

#endif // MYTHREAD_H
