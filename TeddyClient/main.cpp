#include <QApplication>
#include "clientwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("TeddyCorp");
    QApplication::setApplicationName("Teddy's Chat");
    ClientWindow w;
    w.show();
    return app.exec();
}
