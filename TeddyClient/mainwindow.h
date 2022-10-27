#pragma once
#include <QMainWindow>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();
    void on_sendButton_clicked();

    void on_messageField_returnPressed();

public slots:
    void slotReadyRead();
private:
    QTcpSocket *socket;
    QByteArray data;
    Ui::MainWindow *ui;

    void sendToServer(QString str);
};
