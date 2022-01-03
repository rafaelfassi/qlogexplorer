#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<ssize_t>("ssize_t");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
