#include "clientwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ClientWindow w;
    w.setWindowTitle("Client");
    w.resize(300, 500);
    w.show();
    return app.exec();
}
