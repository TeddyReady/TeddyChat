#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
    w.setWindowTitle("User");
    w.resize(300, 500);
    w.show();
    return app.exec();
}
