#include "mainwindow.h"
#include <QApplication>
#include "autorevision.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(VCS_BASENAME);
    a.setApplicationVersion(VCS_TAG);
    MainWindow w;
    w.show();

    return a.exec();
}
