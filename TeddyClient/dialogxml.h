#pragma once
#include <QDomDocument>
#include <QFileDialog>
#include <QDialog>
#include <QTime>

namespace Ui {
class DialogXML;
}

class DialogXML : public QDialog{
    Q_OBJECT
public:
    explicit DialogXML(QWidget *parent = nullptr);
    ~DialogXML();

private slots:
    void on_hideOption_stateChanged(int arg1);
    void on_saveBtn_clicked();
signals:
    void savePath(QString, quint8);
private:
    Ui::DialogXML *ui;
};

