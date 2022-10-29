#include "dialogusername.h"
#include "ui_dialogusername.h"

DialogUserName::DialogUserName(QWidget *parent, QString name) :
    QDialog(parent),
    ui(new Ui::DialogUserName)
{
    ui->setupUi(this);
    ui->nameLine->setText(name);
}

DialogUserName::~DialogUserName() {
    delete ui;
}

void DialogUserName::on_buttonBox_accepted() {
    emit dialogUserNameParams(ui->nameLine->text());
    this->close();
}

void DialogUserName::on_buttonBox_rejected() {
    this->close();
}
