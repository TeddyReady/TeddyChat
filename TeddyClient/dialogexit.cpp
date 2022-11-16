#include "dialogexit.h"
#include "ui_dialogexit.h"

DialogExit::DialogExit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogExit)
{
    ui->setupUi(this);
}

DialogExit::~DialogExit()
{
    delete ui;
}

void DialogExit::on_buttonBox_accepted()
{
    emit closeApplication();
    this->close();
}

void DialogExit::on_buttonBox_rejected()
{
    this->close();
}
