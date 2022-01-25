#include "pch.h"
#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<tp::SInt>("tp::SInt");
    qRegisterMetaType<tp::UInt>("tp::UInt");
    qRegisterMetaType<tp::SharedSIntList>("tp::SharedSIntList");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
