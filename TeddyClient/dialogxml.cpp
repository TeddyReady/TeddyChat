#include "dialogxml.h"
#include "ui_dialogxml.h"

DialogXML::DialogXML(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogXML)
{
    ui->setupUi(this);
    ui->hideOption->setChecked(false);
    ui->fileLine->setText("example");
    ui->passLine->setText("1234");
}

DialogXML::~DialogXML()
{
    delete ui;
}

void DialogXML::on_hideOption_stateChanged(int arg)
{
    Q_UNUSED(arg);
    if(ui->hideOption->isChecked()){
        ui->passLine->setEchoMode(QLineEdit::Password);
    } else {
        ui->passLine->setEchoMode(QLineEdit::Normal);
    }
}

void DialogXML::on_saveBtn_clicked()
{
    QString path = QFileDialog::getSaveFileName(0, QObject::tr("Select saving path"),
                   QDir::homePath() + "/" + ui->fileLine->text().trimmed() + ".xml", QObject::tr("XML files (*.xml)"));
    quint64 pass = (quint64)ui->passLine->text().toInt();
    emit savePath(path, pass);
    this->close();
}
