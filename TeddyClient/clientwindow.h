#pragma once
#include <QMainWindow>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWindow; }
QT_END_NAMESPACE

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void on_connectButton_clicked();
    void on_sendButton_clicked();

    void on_messageField_returnPressed();

public slots:
    void slotReadyRead();
private:
    QTcpSocket *socket;
    QByteArray data;
    Ui::ClientWindow *ui;

    void sendToServer(QString str);
};
