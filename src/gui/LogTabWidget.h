// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QWidget>

class BaseLogModel;
class LogViewWidget;
class LogSearchWidget;
class ProgressLabel;

class LogTabWidget : public QWidget
{
    Q_OBJECT

public:
    // LogTabWidget gets ownership of conf
    LogTabWidget(FileConf::Ptr conf, QWidget *parent = nullptr);
    ~LogTabWidget();
    void retranslateUi();
    void reconfigure();
    void updateConfName();
    FileConf::Ptr getConf();

protected:
    void createActions();
    void createMenus();
    void createToolBars();
    void createConnections();
    void translateUi();

private:
    BaseLogModel *m_logModel;
    LogViewWidget *m_logViewWidget;
    LogSearchWidget *m_logSearchWidget;
    FileConf::Ptr m_conf;
    QAction *m_actTrackFile;
    QAction *m_actAutoScrolling;
    ProgressLabel *m_prlFileParsing;
};
