#pragma once
#include <QDialog>
#include "../myclient.h"

namespace Ui {
class DialogAboutClient;
}

class DialogAboutClient : public QDialog{
    Q_OBJECT
public:
    DialogAboutClient(QWidget *parent, MyClient client);
    ~DialogAboutClient();
private slots:
    void on_pushButton_clicked();
private:
    MyClient client;
    Ui::DialogAboutClient *ui;
};
