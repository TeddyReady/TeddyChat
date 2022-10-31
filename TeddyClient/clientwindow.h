#pragma once
#include <QMainWindow>
//#include <QtXml>
#include <QSslSocket>
#include <QTime>
#include "dialogipport.h"
#include "dialogusername.h"
#include "dialogaboutautor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWindow; }
QT_END_NAMESPACE

enum Status {Online, NotInPlace, NotDisturb};

class ClientWindow : public QMainWindow {
    Q_OBJECT
private:
    QSslSocket *socket;
    QByteArray data;
    QString ip = "127.0.0.1";
    int port = 2075;
    QString username = "Unknown User";
    int status = Status::Online;
    Ui::ClientWindow *ui;

    void sendToServer(QString str);
    void sendToServerDuty(QString str);

public slots:
    void slotReadyRead();

public:
    explicit ClientWindow(QWidget *parent = nullptr);
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
    void on_actionOnline_triggered();
    void on_actionNotInPlace_triggered();
    void on_actionDoNotDisturb_triggered();

    //UI Fields
    void on_sendButton_clicked();
    void on_messageField_returnPressed();

    //Socket slots
    void slotSocketConnected();
    void slotSocketDisconnected();
    void on_appAct_triggered();

};
