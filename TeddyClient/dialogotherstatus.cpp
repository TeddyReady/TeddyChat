#include "dialogotherstatus.h"
#include "ui_dialogotherstatus.h"

DialogOtherStatus::DialogOtherStatus(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOtherStatus)
{
    ui->setupUi(this);
}

DialogOtherStatus::~DialogOtherStatus()
{
    delete ui;
}

void DialogOtherStatus::on_buttonBox_accepted()
{
    emit dialogOtherStatusParams(ui->lineStatus->text());
    this->close();
}

void DialogOtherStatus::on_buttonBox_rejected()
{
    this->close();
}
