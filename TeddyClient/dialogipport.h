#pragma once
#include <QDialog>

namespace Ui {
class DialogIPPort;
}

class DialogIPPort : public QDialog {
    Q_OBJECT
private:
    QString ip;
    int port;
    Ui::DialogIPPort *ui;
public:
    explicit DialogIPPort(QWidget *parent = nullptr, QString ip = "127.0.0.1", int port = 2075);
    ~DialogIPPort();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

signals:
    void dialogIPPortParams(QString, int);
};
