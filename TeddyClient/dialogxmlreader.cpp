#include "dialogxmlreader.h"
#include "ui_dialogxmlreader.h"

DialogXMLReader::DialogXMLReader(QWidget *parent, QString path, quint8 pass, bool encrypted) :
    QDialog(parent),
    ui(new Ui::DialogXMLReader)
{
    ui->setupUi(this);
    Cipher crypto(pass);
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader in(&file);
        if (in.readNextStartElement()) {
            if (in.name() == "history" || in.name() == "logs") {
                QListWidgetItem *item = new QListWidgetItem("History and logs..." , ui->field);
                item->setForeground(Qt::darkGreen);
                ui->field->addItem(item);
                while (in.readNextStartElement()) {
                    if (in.name() == "date" || in.name() == "time" || in.name() == "ip" || in.name() == "username" || in.name() == "message") {
                        if (encrypted) item = new QListWidgetItem(crypto.decryptToString(in.readElementText()) , ui->field);
                        else item = new QListWidgetItem(in.readElementText() , ui->field);
                        item->setForeground(Qt::black);
                        ui->field->addItem(item);
                    }
                }
            }
        }
    } else {
        QListWidgetItem *item = new QListWidgetItem("Error!", ui->field);
        item->setForeground(Qt::darkRed);
        ui->field->addItem(item);
    }
}

DialogXMLReader::~DialogXMLReader()
{
    ui->field->clear();
    delete ui;
}

void DialogXMLReader::on_pushButton_clicked()
{
    close();
}
