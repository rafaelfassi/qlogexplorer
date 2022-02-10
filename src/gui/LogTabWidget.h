// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QWidget>

class BaseLogModel;
class LogViewWidget;
class LogSearchWidget;

class LogTabWidget : public QWidget
{
    Q_OBJECT

public:
    // LogTabWidget gets ownership of conf
    LogTabWidget(Conf::Ptr conf, QWidget *parent = nullptr);
    ~LogTabWidget();
    void updateColumns();
    Conf::Ptr getConf();

protected:
    void createActions();
    void createMenus();
    void createToolBars();
    void createConnections();

private:
    BaseLogModel *m_logModel;
    LogViewWidget *m_logViewWidget;
    LogSearchWidget *m_logSearchWidget;
    Conf::Ptr m_conf;
};
