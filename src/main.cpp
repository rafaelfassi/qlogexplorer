// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "MainWindow.h"
#include "Settings.h"
#include "Style.h"
#include <QApplication>
#include <QFontDatabase>
#include <QStyleFactory>
#include <QStyle>
#include <QCommandLineParser>
#include <QDebug>

int main(int argc, char *argv[])
{
    qRegisterMetaType<tp::SInt>("tp::SInt");
    qRegisterMetaType<tp::UInt>("tp::UInt");
    qRegisterMetaType<tp::SharedSIntList>("tp::SharedSIntList");

    QApplication app(argc, argv);
    QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
    Settings::initSettings();
    Style::initStyle();
    Style::loadStyle(Settings::getStyle());

    qDebug() << Style::availableStyles();

    QCommandLineParser parser;
    QCommandLineOption typeOption(
        QStringList() << "t"
                      << "type",
        QCoreApplication::translate("main", "Opens as <FileTypeOrTemplateName>"),
        "FileTypeOrTemplateName",
        tp::toStr(tp::FileType::Text).c_str());
    parser.addHelpOption();
    parser.addOption(typeOption);
    parser.process(app);

    MainWindow w;
    w.show();

    const QStringList files = parser.positionalArguments();
    if (!files.isEmpty())
    {
        w.setFilesToOpen(files);
        w.openFiles(parser.value(typeOption));
    }

    return app.exec();
}
