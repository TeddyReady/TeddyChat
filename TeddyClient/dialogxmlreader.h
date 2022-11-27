#ifndef DIALOGXMLREADER_H
#define DIALOGXMLREADER_H
#include <QXmlStreamReader>
#include <QListWidgetItem>
#include <QDialog>
#include <QFile>

#include "dialogpassword.h"
#include "cipher.h"

namespace Ui {
class DialogXMLReader;
}

class DialogXMLReader : public QDialog {
    Q_OBJECT
public:
    explicit DialogXMLReader(QWidget *parent = nullptr, QString path = "", quint8 pass = static_cast<quint8>(1234), bool encrypted = true);
    ~DialogXMLReader();
private slots:
    void on_pushButton_clicked();
private:
    Ui::DialogXMLReader *ui;
};

#endif // DIALOGXMLREADER_H
