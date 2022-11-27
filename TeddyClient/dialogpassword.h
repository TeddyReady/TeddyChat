#ifndef DIALOGPASSWORD_H
#define DIALOGPASSWORD_H
#include <QDialog>

namespace Ui {
class DialogPassword;
}

class DialogPassword : public QDialog {
    Q_OBJECT
public:
    explicit DialogPassword(QWidget *parent = nullptr, QString path = "");
    ~DialogPassword();
signals:
    void passwordReceived(QString, quint8, bool);
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_btnEncrypt_clicked();

private:
    QString path;
    Ui::DialogPassword *ui;
};

#endif // DIALOGPASSWORD_H
