#pragma once
#include <QDialog>

namespace Ui {
class DialogExit;
}

class DialogExit : public QDialog{
    Q_OBJECT
public:
    explicit DialogExit(QWidget *parent = nullptr);
    ~DialogExit();
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
private:
    Ui::DialogExit *ui;
signals:
    void closeApplication();
};
