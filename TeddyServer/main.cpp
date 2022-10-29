#include <QApplication>
#include "serverwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ServerWindow w;
    w.setWindowTitle("Server");
    w.resize(400, 600);
    w.show();
    return app.exec();
}
