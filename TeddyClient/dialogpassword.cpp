#include "dialogpassword.h"
#include "ui_dialogpassword.h"

DialogPassword::DialogPassword(QWidget *parent, QString path) :
    QDialog(parent),
    ui(new Ui::DialogPassword)
{
    ui->setupUi(this);
    this->path = path;
    ui->linePass->setText("1234");
    ui->btnEncrypt->setChecked(true);
}

DialogPassword::~DialogPassword()
{
    delete ui;
}

void DialogPassword::on_buttonBox_accepted()
{
    emit passwordReceived(path, static_cast<quint8>(ui->linePass->text().toInt()), ui->btnEncrypt->isChecked());
    close();
}

void DialogPassword::on_buttonBox_rejected()
{
    close();
}

void DialogPassword::on_btnEncrypt_clicked()
{
    if (ui->btnEncrypt->isChecked())
        ui->linePass->setDisabled(false);
    else ui->linePass->setEnabled(false);
}
