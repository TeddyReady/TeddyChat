#pragma once
#include <QFileDialog>
#include <QDialog>

#include "constants.h"

namespace Ui {
class DialogImage;
}

class DialogImage : public QDialog{
    Q_OBJECT
public:
    explicit DialogImage(QWidget *parent, QPixmap image);
    ~DialogImage();
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
private:
    Ui::DialogImage *ui;
signals:
    void saveImage(QString, const QPixmap *);
};
