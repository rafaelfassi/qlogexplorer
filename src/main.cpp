// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "MainWindow.h"
#include "Settings.h"
#include "Style.h"
#include <QApplication>
#include <QtSingleApplication>
#include <QStyleFactory>
#include <QStyle>
#include <QCommandLineParser>
#include <QJsonDocument>
#include <QDebug>

int main(int argc, char *argv[])
{
    qRegisterMetaType<tp::SInt>("tp::SInt");
    qRegisterMetaType<tp::UInt>("tp::UInt");
    qRegisterMetaType<tp::SharedSIntList>("tp::SharedSIntList");

    QtSingleApplication app(argc, argv);

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

    QVariantMap cmdArgsMap;
    cmdArgsMap.insert("files", QVariant(parser.positionalArguments()));
    cmdArgsMap.insert("type", QVariant(parser.value(typeOption)));

    Settings::initSettings();

    app.initialize(QString(), Settings::getSettingsDir().absolutePath());

    if (Settings::getSingleInstance())
    {
        auto json = QJsonDocument::fromVariant(cmdArgsMap);
        if (app.sendMessage(json.toJson()))
        {
            LOG_INF("Request was forwarded to the already running instance");
            return 0;
        }
    }

    Style::initStyle();
    Style::loadStyle(Settings::getStyle());

    qDebug() << Style::availableStyles();

    MainWindow w;
    w.show();

    app.setActivationWindow(&w);

    const auto openFilesFunc = [&w](const QVariantMap &argsMap)
    {
        const auto &type = argsMap["type"].toString();
        const auto &files = argsMap["files"].toStringList();
        if (!type.isEmpty() && !files.isEmpty())
        {
            w.setFilesToOpen(files);
            w.openFiles(type);
        }
    };

    QObject::connect(
        &app,
        &QtSingleApplication::messageReceived,
        &w,
        [&openFilesFunc](const QString &otherArgs)
        {
            const auto otherArgsMap = QJsonDocument::fromJson(otherArgs.toUtf8()).toVariant().toMap();
            openFilesFunc(otherArgsMap);
        });

    openFilesFunc(cmdArgsMap);

    return app.exec();
}
