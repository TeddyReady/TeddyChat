#pragma once
#include <QColorDialog>
#include <QCloseEvent>
#include <QMainWindow>
#include <QSslSocket>
#include <QSettings>
#include <QBuffer>
#include <QVector>
#include <QSound>

#include "dialogotherstatus.h"
#include "dialogaboutclient.h"
#include "dialogaboutautor.h"
#include "dialogxmlreader.h"
#include "dialogusername.h"
#include "dialogipport.h"
#include "dialogimage.h"
#include "dialogexit.h"
#include "dialogxml.h"
#include "md5.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWindow; }
QT_END_NAMESPACE

class ClientWindow : public QMainWindow {
    Q_OBJECT
private:
    //Chat
    MyClient client;
    bool isConnected, showIP, showTime;
    quint64 dataSize;
    QVector<MyClient *> includedClients;
    //XML
    QFile file;
    QDomDocument doc;
    QDomElement general, date, time, ip, name, message;
    QVector<QString> xmlData;
    //Color
    QPalette pal;
    QColor windowColor, myMsgColor, otherMsgColor;
    //Other
    QVector<QString> fileNames;
    QSettings *settings;
    Ui::ClientWindow *ui;

    void sendToServer(int command, QString message = "", int option = 0);
protected:
    void closeEvent(QCloseEvent *event);
public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

    //Для сохранения и установки настроек в/из ini файл(а)
    void uploadSettings();
    void saveSettings();
public slots:
    void slotReadyRead();
    void slotEncrypted();
private slots:
    //Меню "General"
    void on_connectAct_triggered();
    void on_disconnectAct_triggered();
    void on_saveHistoryAct_triggered();
    void slotSavePath(QString, quint8);
    void on_actionReadXML_triggered();
    void slotPasswordReceived(QString, quint8, bool);
    void on_quitAct_triggered();
    void slotCloseApplication();

    //Меню "View"
    void on_actionBackColor_triggered();
    void on_actionMyColorMsg_triggered();
    void on_actionOtherColorMsg_triggered();
    void on_actionProfileColor_triggered();
    void on_actionShowIP_triggered();
    void on_actionShowTime_triggered();

    //Меню "Settings"
    void on_ipPortAct_triggered();
    void slotDialogIPPortParams(QString, int);
    void on_actionAvatar_triggered();
    void on_nameAct_triggered();
    void slotDialogUserNameParams(QString);
    void on_actionOnline_triggered();
    void on_actionNotInPlace_triggered();
    void on_actionDoNotDisturb_triggered();
    void on_actionOther_triggered();
    void slotDialogOtherStatusParams(QString);

    //Отправка СМС
    void on_sendButton_clicked();
    void on_messageField_returnPressed();

    //Слоты сокета
    void slotSocketConnected();
    void slotSocketDisconnected();
    void on_appAct_triggered();

    //UI
    void on_profileButton_clicked();
    void showContextMenu(QPoint);
    void slotInfoAbout();
    void showContextMenuOnMessageField(QPoint);
    void slotSaveImage(QString, const QPixmap *);
    void showContextMenuOnSendButton(QPoint);
    void slotSendPicture();
    void slotSendFile();
    void on_actionIlya_triggered();
    void on_actionJasmin_triggered();
    void on_actionBoris_triggered();
    void on_actionAnastasia_triggered();
};
