#pragma once
#include <QMainWindow>
#include <QTime>
#include "myserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ServerWindow; }
QT_END_NAMESPACE

class ServerWindow : public QMainWindow {
    Q_OBJECT
public:
    ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();
private:
    MyServer *server;
    Ui::ServerWindow *ui;
public slots:
    void slotNewConnection(QString);
    void slotClientDisconnected(QString);
};
