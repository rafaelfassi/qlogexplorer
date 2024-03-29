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
#include "ProgressLabel.h"
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

    m_prlSearching = new ProgressLabel(this);

    m_proxyModel = new ProxyModel(m_sourceModel);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(btnMergeResults);
    hLayout->addWidget(btnOrOperator);
    hLayout->addWidget(btnClear);
    hLayout->addWidget(btnSyncMarks);
    hLayout->addWidget(btnExec);
    hLayout->addWidget(m_prlSearching);
    hLayout->addWidget(btnAddSearchParam);

    m_searchResults = new LogViewWidget(m_proxyModel, mainLog->getMarkedTexts(), this);

    m_searchParamsLayout = new QVBoxLayout();
    m_searchParamsLayout->setMargin(2);
    m_searchParamsLayout->setSpacing(2);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(m_searchParamsLayout);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_searchResults);

    translateUi();

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
    for (auto paramWidget : qAsConst(m_searchParamWidgets))
    {
        paramWidget->reconfigure();
    }
    m_searchResults->reconfigure(m_conf);
    m_searchParamModel->updateParams(m_conf->getFilterParams());
}

void LogSearchWidget::createActions()
{
    m_actAddSearchParam = new QAction(this);

    m_actMergeResults = new QAction(this);
    m_actMergeResults->setCheckable(true);

    m_actOrOperator = new QAction(this);
    m_actOrOperator->setCheckable(true);

    m_actClear = new QAction(this);

    m_actSyncMarks = new QAction(this);

    m_actExec = new QAction(this);
}

void LogSearchWidget::translateUi()
{
    m_actAddSearchParam->setText(tr("Add Search Parameter"));
    m_actAddSearchParam->setIcon(Style::getIcon("add_icon.png"));

    m_actMergeResults->setText(tr("Merge Results"));
    m_actMergeResults->setIcon(Style::getIcon("merge_icon.png"));

    m_actOrOperator->setText(tr("Use OR operator"));
    m_actOrOperator->setIcon(Style::getIcon("or_icon.png"));

    m_actClear->setText(tr("Clear Results"));
    m_actClear->setIcon(Style::getIcon("clear_icon.png"));

    m_actSyncMarks->setText(tr("Sync Bookmarks"));
    m_actSyncMarks->setIcon(Style::getIcon("sync_icon.png"));

    m_actExec->setText(tr("Search"));
    m_actExec->setIcon(Style::getIcon("search_icon.png"));

    m_prlSearching->setActionText(tr("Searching"));
}

void LogSearchWidget::retranslateUi()
{
    translateUi();
    for (auto paramWidget : qAsConst(m_searchParamWidgets))
    {
        paramWidget->retranslateUi();
    }
    m_searchResults->retranslateUi();
}

void LogSearchWidget::createConnections()
{
    connect(m_actExec, &QAction::triggered, this, &LogSearchWidget::startSearch);
    connect(m_actClear, &QAction::triggered, this, &LogSearchWidget::clearResults);
    connect(m_actSyncMarks, &QAction::triggered, this, &LogSearchWidget::syncMarks);
    connect(m_actAddSearchParam, &QAction::triggered, this, &LogSearchWidget::addSearchParam);
    connect(m_sourceModel, &BaseLogModel::modelConfigured, this, &LogSearchWidget::sourceModelConfigured);
    connect(m_sourceModel, &BaseLogModel::valueFound, this, &LogSearchWidget::addSearchResult);
    connect(m_sourceModel, &BaseLogModel::searchingProgressChanged, m_prlSearching, &ProgressLabel::setProgress);
    connect(m_searchResults, &LogViewWidget::rowSelected, m_mainLog, &LogViewWidget::goToRow);
    connect(m_searchResults, &LogViewWidget::textMarkUpdated, m_mainLog, QOverload<>::of(&LogViewWidget::update));
    connect(m_mainLog, &LogViewWidget::textMarkUpdated, m_searchResults, QOverload<>::of(&LogViewWidget::update));
}

void LogSearchWidget::addSearchParam()
{
    SearchParamWidget *param = new SearchParamWidget(m_conf, m_searchParamModel, this);

    auto paramSizePolicy = param->sizePolicy();
    paramSizePolicy.setVerticalPolicy(QSizePolicy::Minimum);
    param->setSizePolicy(paramSizePolicy);
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

    for (auto paramWidget : qAsConst(m_searchParamWidgets))
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

    m_searchResults->clearBookmarks();
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
