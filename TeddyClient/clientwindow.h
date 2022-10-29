#pragma once
#include <QMainWindow>
//#include <QtXml>
#include <QTcpSocket>
#include "dialogipport.h"
#include "dialogusername.h"
#include "dialogstatus.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWindow; }
QT_END_NAMESPACE

class ClientWindow : public QMainWindow {
    Q_OBJECT
private:
    QTcpSocket *socket;
    QByteArray data;
    QString ip = "127.0.0.1";
    int port = 2075;
    QString username = "Unknown User";
    int status = Status::Online;
    Ui::ClientWindow *ui;

    void sendToServer(QString str);

public slots:
    void slotReadyRead();

public:
    ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    //Menu "File"
    void on_connectAct_triggered();
    void on_disconnectAct_triggered();
    void on_saveHistoryAct_triggered();
    void on_quitAct_triggered();

    //Menu "Settings"
    void on_ipPortAct_triggered();
    void slotDialogIPPortParams(QString, int);
    void on_nameAct_triggered();
    void slotDialogUserNameParams(QString);
    void on_statusAct_triggered();
    void slotDialogStatusParams(int);

    //UI Fields
    void on_sendButton_clicked();
    void on_messageField_returnPressed();

    //Socket slots
    void slotSocketConnected();
    void slotSocketDisconnected();
};
