#pragma once
#include <QDialog>

namespace Ui {
class DialogOtherStatus;
}

class DialogOtherStatus : public QDialog{
    Q_OBJECT
public:
    explicit DialogOtherStatus(QWidget *parent = nullptr);
    ~DialogOtherStatus();
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
signals:
    void dialogOtherStatusParams(QString);
private:
    Ui::DialogOtherStatus *ui;
};
