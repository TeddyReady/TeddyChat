#pragma once
#include <QMainWindow>
//#include <QtXml>
#include <QSslSocket>
#include <QTime>
#include <QSettings>
#include <QSound>
#include "myclient.h"
#include "dialogipport.h"
#include "dialogusername.h"
#include "dialogaboutautor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWindow; }
QT_END_NAMESPACE

class ClientWindow : public QMainWindow {
    Q_OBJECT
private:
    MyClient client;

    QSettings *settings;
    Ui::ClientWindow *ui;

    void sendDataToServer();
    void sendMessageToServer(QString str);
public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

    //Для сохранения и установки настроек в/из ini файл(а)
    void uploadSettings();
    void saveSettings();

public slots:
    void slotReadyRead();

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
