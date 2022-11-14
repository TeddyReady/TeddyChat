#include "dialogaboutclient.h"
#include "ui_dialogaboutclient.h"

DialogAboutClient::DialogAboutClient(QWidget *parent, MyClient client) :
    QDialog(parent), client(client),
    ui(new Ui::DialogAboutClient)
{
    ui->setupUi(this);
    ui->nameLabel->setText(client.username);
    switch(client.status){
    case Status::Online:
        ui->statusLabel->setText("Online");
        break;
    case Status::NotInPlace:
        ui->statusLabel->setText("Not In Place");;
        break;
    case Status::NotDisturb:
        ui->statusLabel->setText("Do Not Disturb");
        break;
    case Status::Other:
        ui->statusLabel->setText(client.statusName);
        break;
    }
    ui->dataLabel->setText(client.date);
    ui->labelIn->setText("in");
    ui->timeLabel->setText(client.time);
}

DialogAboutClient::~DialogAboutClient()
{
    delete ui;
}

void DialogAboutClient::on_pushButton_clicked()
{
    this->close();
}
