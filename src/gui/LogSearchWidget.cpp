// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "LogSearchWidget.h"
#include "LogViewWidget.h"
#include "BaseLogModel.h"
#include "SearchParamModel.h"
#include "SearchParamWidget.h"
#include "ProxyModel.h"
#include "LongScrollBar.h"
#include "Style.h"
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QToolButton>

LogSearchWidget::LogSearchWidget(FileConf::Ptr conf, LogViewWidget *mainLog, BaseLogModel *sourceModel, QWidget *parent)
    : QWidget(parent),
      m_conf(conf),
      m_mainLog(mainLog),
      m_sourceModel(sourceModel)
{
    createActions();

    QToolButton *btnAddSearchParam = new QToolButton(this);
    btnAddSearchParam->setFocusPolicy(Qt::NoFocus);
    btnAddSearchParam->setDefaultAction(m_actAddSearchParam);

    QToolButton *btnMergeResults = new QToolButton(this);
    btnMergeResults->setFocusPolicy(Qt::NoFocus);
    btnMergeResults->setDefaultAction(m_actMergeResults);

    QToolButton *btnOrOperator = new QToolButton(this);
    btnOrOperator->setFocusPolicy(Qt::NoFocus);
    btnOrOperator->setDefaultAction(m_actOrOperator);

    QToolButton *btnClear = new QToolButton(this);
    btnClear->setFocusPolicy(Qt::NoFocus);
    btnClear->setDefaultAction(m_actClear);

    QToolButton *btnSyncMarks = new QToolButton(this);
    btnSyncMarks->setFocusPolicy(Qt::NoFocus);
    btnSyncMarks->setDefaultAction(m_actSyncMarks);

    QToolButton *btnExec = new QToolButton(this);
    btnExec->setFocusPolicy(Qt::NoFocus);
    btnExec->setDefaultAction(m_actExec);

    m_proxyModel = new ProxyModel(m_sourceModel);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(btnMergeResults);
    hLayout->addWidget(btnOrOperator);
    hLayout->addWidget(btnClear);
    hLayout->addWidget(btnSyncMarks);
    hLayout->addWidget(btnExec);
    hLayout->addStretch();
    hLayout->addWidget(btnAddSearchParam);

    m_searchResults = new LogViewWidget(m_proxyModel, this);

    m_searchParamsLayout = new QVBoxLayout();
    m_searchParamsLayout->setMargin(2);
    m_searchParamsLayout->setSpacing(2);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(m_searchParamsLayout);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_searchResults);

    setLayout(vLayout);

    m_searchParamModel = new SearchParamModel(this);

    createConnections();

    addSearchParam();
}

LogSearchWidget::~LogSearchWidget()
{
}

void LogSearchWidget::configure()
{
    m_searchResults->configure(m_conf);
    m_searchParamModel->loadParams(m_conf->getFilterParams());
}

void LogSearchWidget::reconfigure()
{
    for (auto paramWidget : m_searchParamWidgets)
    {
        paramWidget->updateColumns();
    }
    m_searchResults->resetColumns();
    m_searchResults->configure(m_conf);
    m_searchParamModel->updateParams(m_conf->getFilterParams());
}

void LogSearchWidget::createActions()
{
    m_actAddSearchParam = new QAction(tr("Add Search Parameter"), this);
    m_actAddSearchParam->setIcon(Style::getIcon("add_icon.png"));

    m_actMergeResults = new QAction(tr("Merge Results"), this);
    m_actMergeResults->setIcon(Style::getIcon("merge_icon.png"));
    m_actMergeResults->setCheckable(true);

    m_actOrOperator = new QAction(tr("Use OR operator"), this);
    m_actOrOperator->setIcon(Style::getIcon("or_icon.png"));
    m_actOrOperator->setCheckable(true);

    m_actClear = new QAction(tr("Clear Results"), this);
    m_actClear->setIcon(Style::getIcon("clear_icon.png"));

    m_actSyncMarks = new QAction(tr("Sync Bookmarks and Marks"), this);
    m_actSyncMarks->setIcon(Style::getIcon("sync_icon.png"));

    m_actExec = new QAction(tr("Search"), this);
    m_actExec->setIcon(Style::getIcon("search_icon.png"));
}

void LogSearchWidget::createConnections()
{
    connect(m_actExec, &QAction::triggered, this, &LogSearchWidget::startSearch);
    connect(m_actClear, &QAction::triggered, this, &LogSearchWidget::clearResults);
    connect(m_actSyncMarks, &QAction::triggered, this, &LogSearchWidget::syncMarks);
    connect(m_actAddSearchParam, &QAction::triggered, this, &LogSearchWidget::addSearchParam);
    connect(m_sourceModel, &BaseLogModel::modelConfigured, this, &LogSearchWidget::sourceModelConfigured);
    connect(m_sourceModel, &BaseLogModel::valueFound, this, &LogSearchWidget::addSearchResult);
    connect(m_searchResults, &LogViewWidget::rowSelected, m_mainLog, &LogViewWidget::goToRow);
}

void LogSearchWidget::addSearchParam()
{
    SearchParamWidget *param = new SearchParamWidget(m_conf, m_searchParamModel, this);
    param->sizePolicy().setVerticalPolicy(QSizePolicy::Minimum);
    connect(param, &SearchParamWidget::searchRequested, this, &LogSearchWidget::startSearch);
    connect(param, &SearchParamWidget::deleteRequested, this, &LogSearchWidget::deleteParamWidget);
    m_searchParamWidgets.append(param);
    m_searchParamsLayout->addWidget(param);
}

void LogSearchWidget::startSearch()
{
    if (!m_actMergeResults->isChecked())
    {
        clearResults();
    }

    tp::SearchParams params;

    for (auto paramWidget : m_searchParamWidgets)
    {
        if (paramWidget->getIsEnabled())
        {
            paramWidget->apply();
            auto param = paramWidget->getSearchParam();
            if (!param.pattern.empty())
            {
                params.push_back(std::move(param));
            }
        }
    }

    if (!params.empty())
    {
        m_sourceModel->startSearch(params, m_actOrOperator->isChecked());
    }
    else
    {
        m_sourceModel->stopSearch();
    }
}

void LogSearchWidget::addSearchResult(tp::SharedSIntList rowsPtr)
{
    if (m_sourceModel->isSearching())
    {
        m_proxyModel->addSourceRows(*rowsPtr.get());
        m_searchResults->updateView();
    }
}

void LogSearchWidget::clearResults()
{
    m_sourceModel->stopSearch();
    m_proxyModel->clear();
    m_searchResults->clearBookmarks();
    m_searchResults->updateView();
}

void LogSearchWidget::deleteParamWidget(QWidget *paramWidget)
{
    for (tp::UInt i = 0; i < m_searchParamWidgets.size(); ++i)
    {
        if (m_searchParamWidgets.at(i) == paramWidget)
        {
            m_searchParamWidgets.removeAt(i);
            paramWidget->deleteLater();
            return;
        }
    }
}

void LogSearchWidget::sourceModelConfigured()
{
    reconfigure();
}

void LogSearchWidget::syncMarks()
{
    for (const auto searchResultsRow : m_searchResults->getBookmarks())
    {
        m_mainLog->addBookmark(m_proxyModel->getRowNum(searchResultsRow));
    }

    for (const auto mainLogRow : m_mainLog->getBookmarks())
    {
        m_proxyModel->addSourceRow(mainLogRow);
        const auto row = m_proxyModel->findSourceRow(mainLogRow);
        if (row != -1)
        {
            m_searchResults->addBookmark(row);
        }
    }
    m_mainLog->updateView();
    m_searchResults->updateView();
}