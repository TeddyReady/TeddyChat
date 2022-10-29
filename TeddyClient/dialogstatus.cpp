#include "dialogstatus.h"
#include "ui_dialogstatus.h"

DialogStatus::DialogStatus(QWidget *parent, int status) :
    QDialog(parent),
    ui(new Ui::DialogStatus)
{
    ui->setupUi(this);
    if (status == Status::Online) {
        ui->statusOnline->setChecked(true);
    } else if (status == Status::NotInPlace) {
        ui->statusNotInPlace->setChecked(true);
    } else {
        ui->statusNotDisturb->setChecked(true);
    }
}

DialogStatus::~DialogStatus() {
    delete ui;
}

void DialogStatus::on_buttonBox_accepted() {
    if (ui->statusOnline->isChecked()) {
        emit dialogStatusParams(Status::Online);
    } else if (ui->statusNotInPlace->isChecked()) {
        emit dialogStatusParams(Status::NotInPlace);
    } else {
        emit dialogStatusParams(Status::NotDisturb);
    }
    this->close();
}

void DialogStatus::on_buttonBox_rejected() {
    this->close();
}
