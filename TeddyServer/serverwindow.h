#pragma once
#include <QCloseEvent>
#include <QMainWindow>
#include <QSettings>
#include <QVector>
#include <QTime>

#include "dialogipport.h"
#include "dialogexit.h"
#include "myserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ServerWindow; }
QT_END_NAMESPACE

class ServerWindow : public QMainWindow {
    Q_OBJECT
private:
    MyServer *server;

    QSettings *settings;
    Ui::ServerWindow *ui;
protected:
    void closeEvent(QCloseEvent *event);
public:
    ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

    //Для сохранения и установки настроек в/из ini файл(а)
    void uploadSettings();
    void saveSettings();
public slots:
    void slotServerStatus(bool online);
    void slotNewConnection(MyClient *client);
    void slotFailedValidation();
    void slotClientDisconnected(MyClient *client);
    void slotReNameOnUI(QString name, QString newName);
    //Windows
    void slotDialogIPPortParams(QString ip,int port);
private slots:
    //General
    void on_actionDeploy_triggered();
    void on_actionReload_triggered();
    void on_actionStop_triggered();
    void on_actionExit_triggered();
    void slotCloseApplication();
    //Settings
    void on_actionNetwork_triggered();
};
