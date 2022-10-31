#include "dialogaboutautor.h"
#include "ui_dialogaboutautor.h"

DialogAboutAutor::DialogAboutAutor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAboutAutor)
{
    ui->setupUi(this);
    ui->dataLabel->setText(curDate.toString("dd/MM/yy"));
    ui->qtLabel->setText(QT_VERSION_STR);
}

DialogAboutAutor::~DialogAboutAutor()
{
    delete ui;
}

void DialogAboutAutor::on_pushButton_clicked()
{
    this->close();
}
