//#include "mainwindow.h"
#include "harmonic.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // MainWindow w;
    // w.show();

    harmonic harmonicWin;
    harmonicWin.show();

    return a.exec();
}
