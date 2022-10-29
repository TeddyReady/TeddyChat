#pragma once
#include <QMainWindow>
#include <QtXml>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWindow; }
QT_END_NAMESPACE

class ClientWindow : public QMainWindow {
    Q_OBJECT
private:
    QTcpSocket *socket;
    QByteArray data;
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

    //UI Fields
    void on_sendButton_clicked();
    void on_messageField_returnPressed();

    //Socket slots
    void slotSocketConnected();
    void slotSocketDisconnected();
};
