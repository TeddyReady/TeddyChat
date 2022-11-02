#pragma once
#include <QDialog>

namespace Ui {
class DialogUserName;
}

class DialogUserName : public QDialog {
    Q_OBJECT
private:
    QString name;
    Ui::DialogUserName *ui;
public:
    explicit DialogUserName(QWidget *parent = nullptr, QString name = "Unknown User");
    ~DialogUserName();
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
signals:
    void dialogUserNameParams(QString);
};
