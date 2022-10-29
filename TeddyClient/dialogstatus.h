#pragma once
#include <QDialog>

namespace Ui {
class DialogStatus;
}

enum Status {Online, NotInPlace, NotDisturb};

class DialogStatus : public QDialog {
    Q_OBJECT
private:
    int status;
    Ui::DialogStatus *ui;

public:
    explicit DialogStatus(QWidget *parent = nullptr, int status = Status::Online);
    ~DialogStatus();
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

signals:
    void dialogStatusParams(int);
};
