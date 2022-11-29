#include <QApplication>
#include "serverwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("TeddyCorp");
    QApplication::setApplicationName("Teddy's Server");
    ServerWindow w;
    w.setWindowTitle("TeddyServer");
    w.show();
    return app.exec();
}
