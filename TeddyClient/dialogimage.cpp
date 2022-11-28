#include "dialogimage.h"
#include "ui_dialogimage.h"

DialogImage::DialogImage(QWidget *parent, QPixmap image) :
    QDialog(parent),
    ui(new Ui::DialogImage)
{
    ui->setupUi(this);
    ui->imageLabel->setPixmap(image);
}

DialogImage::~DialogImage()
{
    delete ui;
}

void DialogImage::on_pushButton_clicked()
{
    QString path = QFileDialog::getSaveFileName(0, QObject::tr("Select saving path"),
                   downloadPath, QObject::tr("Image files (*.png)"));
    if (path != "")
        emit saveImage(path, ui->imageLabel->pixmap());
    this->close();
}

void DialogImage::on_pushButton_2_clicked()
{
    this->close();
}
