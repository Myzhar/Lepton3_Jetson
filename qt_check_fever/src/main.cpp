#include "mainwindow.h"

#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);    

    QSurfaceFormat fmt;
    fmt.setSamples(10);
    QSurfaceFormat::setDefaultFormat(fmt);

    MainWindow w;
    w.show();
    return a.exec();
}
