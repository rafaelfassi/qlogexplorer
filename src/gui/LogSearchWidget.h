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

class LogSearchWidget : public QWidget
{
    Q_OBJECT

public:
    LogSearchWidget(LogViewWidget *mainLog, BaseLogModel *sourceModel, QWidget *parent = nullptr);
    ~LogSearchWidget();
    void configure(Conf *conf);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createConnections();

signals:
    void rowSelected(tp::SInt row);

public slots:
    void addSearchResult(tp::SharedSIntList rowsPtr);
    void resetColumns();

private slots:
    void addSearchParam();
    void startSearch();
    void clearResults();
    void deleteParamWidget(QWidget *);
    void sourceModelConfigured();
    void syncMarks();

private:
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
    QList<SearchParamWidget *> m_searchParamWidgets;
};
