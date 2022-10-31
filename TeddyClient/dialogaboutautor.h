#pragma once
#include <QDialog>
#include <QDate>

namespace Ui {
class DialogAboutAutor;
}

class DialogAboutAutor : public QDialog {
    Q_OBJECT
private:
    QDate curDate = QDate::currentDate();
    Ui::DialogAboutAutor *ui;

public:
    explicit DialogAboutAutor(QWidget *parent = nullptr);
    ~DialogAboutAutor();

private slots:
    void on_pushButton_clicked();
};
