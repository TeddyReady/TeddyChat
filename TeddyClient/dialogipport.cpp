#include "dialogipport.h"
#include "ui_dialogipport.h"

DialogIPPort::DialogIPPort(QWidget *parent, QString ip, int port) :
    QDialog(parent),
    ui(new Ui::DialogIPPort)
{
    ui->setupUi(this);
    ui->ipLine->setText(ip);
    ui->portLine->setText(QString::number(port));
}

DialogIPPort::~DialogIPPort() {
    delete ui;
}

void DialogIPPort::on_buttonBox_accepted() {
    emit dialogIPPortParams(ui->ipLine->text(), ui->portLine->text().toInt());
    this->close();
}

void DialogIPPort::on_buttonBox_rejected() {
    this->close();
}
