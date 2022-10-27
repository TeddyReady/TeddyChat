#include <QApplication>
#include "mainwindow.h"
#include "myserver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyServer server;
    MainWindow w;
    w.show();
    return a.exec();
}
