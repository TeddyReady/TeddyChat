#pragma once
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QMainWindow>
#include <QSslSocket>
#include <QSettings>
#include <QVector>
#include <QSound>
#include <QTime>
#include <QFile>

#include "dialogotherstatus.h"
#include "dialogaboutclient.h"
#include "dialogaboutautor.h"
#include "dialogusername.h"
#include "dialogipport.h"
#include "dialogexit.h"
#include "dialogxml.h"
#include "cipher.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWindow; }
QT_END_NAMESPACE

class ClientWindow : public QMainWindow {
    Q_OBJECT
private:
    //Chat
    MyClient client;
    quint16 dataSize;
    QVector<MyClient *> includedClients;
    //XML
    QFile file;
    QDomDocument doc;
    QDomElement general;
    QDomElement date;
    QDomElement time;
    QDomElement ip;
    QDomElement name;
    QDomElement message;
    QVector<QString> xmlData;
    //Menu on sendButton
    QMenu *btnMenu;
    QAction* sendMsg;
    QAction* sendPic;
    //Other
    QSettings *settings;
    Ui::ClientWindow *ui;

    void sendToServer(int command, QString message = "", int option = 0);
public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

    //Для сохранения и установки настроек в/из ini файл(а)
    void uploadSettings();
    void saveSettings();

    void reConnection();
protected:
    void mousePressEvent(QMouseEvent *event);
public slots:
    void slotReadyRead();
    void slotEncrypted();
private slots:
    //Меню "General"
    void on_connectAct_triggered();
    void on_disconnectAct_triggered();
    void on_saveHistoryAct_triggered();
    void slotSavePath(QString, quint64);
    void on_quitAct_triggered();

    //Меню "Settings"
    void on_ipPortAct_triggered();
    void slotDialogIPPortParams(QString, int);
    void on_nameAct_triggered();
    void slotDialogUserNameParams(QString);
    void on_actionOnline_triggered();
    void on_actionNotInPlace_triggered();
    void on_actionDoNotDisturb_triggered();
    void slotDialogOtherStatusParams(QString);

    //Отправка СМС
    void on_sendButton_clicked();
    void on_messageField_returnPressed();

    //Слоты сокета
    void slotSocketConnected();
    void slotSocketDisconnected();
    void on_appAct_triggered();

    //Меню sendButton
    void slotSendMessage(bool);
    void slotSendPicture(bool);

    //UI
    void on_profileButton_clicked();
    void showContextMenu(QPoint);
    void slotInfoAbout();
    void showContextMenuOnMessageField(QPoint);
    void on_actionOther_triggered();
};
