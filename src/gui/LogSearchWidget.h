// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QWidget>

class QVBoxLayout;
class QAction;
class QComboBox;
class LogViewWidget;
class BaseLogModel;
class ProxyModel;
class SearchParamWidget;
class SearchParamModel;
class ProgressLabel;

class LogSearchWidget : public QWidget
{
    Q_OBJECT

public:
    LogSearchWidget(FileConf::Ptr conf, LogViewWidget *mainLog, BaseLogModel *sourceModel, QWidget *parent = nullptr);
    ~LogSearchWidget();
    void configure();
    void reconfigure();
    void retranslateUi();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createConnections();
    void translateUi();

signals:
    void rowSelected(tp::SInt row);

public slots:
    void addSearchResult(tp::SharedSIntList rowsPtr);

private slots:
    void addSearchParam();
    void startSearch();
    void clearResults();
    void deleteParamWidget(QWidget *);
    void sourceModelConfigured();
    void syncMarks();

private:
    FileConf::Ptr m_conf;
    QAction *m_actAddSearchParam;
    QAction *m_actMergeResults;
    QAction *m_actOrOperator;
    QAction *m_actClear;
    QAction *m_actSyncMarks;
    QAction *m_actExec;
    QVBoxLayout *m_searchParamsLayout;
    LogViewWidget *m_mainLog;
    BaseLogModel *m_sourceModel;
    LogViewWidget *m_searchResults;
    ProxyModel *m_proxyModel;
    ProgressLabel *m_prlSearching;
    SearchParamModel *m_searchParamModel;
    QList<SearchParamWidget *> m_searchParamWidgets;
};
