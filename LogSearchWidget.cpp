#include "pch.h"
#include "LogSearchWidget.h"
#include "LogViewWidget.h"
#include "TextLogModel.h"
#include "JsonLogModel.h"
#include "SearchParamWidget.h"
#include "ProxyModel.h"
#include "LongScrollBar.h"
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QAction>
#include <QToolBar>
#include <QMenuBar>
#include <QLineEdit>
#include <QToolButton>

#include <fstream>

LogSearchWidget::LogSearchWidget(BaseLogModel *sourceModel, QWidget *parent)
    : QWidget(parent),
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

    QToolButton *btnExec = new QToolButton(this);
    btnExec->setFocusPolicy(Qt::NoFocus);
    btnExec->setDefaultAction(m_actExec);

    m_proxyModel = new ProxyModel(m_sourceModel);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(btnMergeResults);
    hLayout->addWidget(btnOrOperator);
    hLayout->addWidget(btnClear);
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

    createConnections();
}

LogSearchWidget::~LogSearchWidget()
{
}

void LogSearchWidget::createActions()
{
    m_actAddSearchParam = new QAction(tr("Add Search Parameter"), this);
    m_actAddSearchParam->setIcon(QIcon(":/images/add_icon.png"));

    m_actMergeResults = new QAction(tr("Merge Results"), this);
    m_actMergeResults->setIcon(QIcon(":/images/merge_icon.png"));
    m_actMergeResults->setCheckable(true);

    m_actOrOperator = new QAction(tr("Use OR operator"), this);
    m_actOrOperator->setIcon(QIcon(":/images/or_icon.png"));
    m_actOrOperator->setCheckable(true);

    m_actClear = new QAction(tr("Clear Results"), this);
    m_actClear->setIcon(QIcon(":/images/clear_icon.png"));

    m_actExec = new QAction(tr("Search"), this);
    m_actExec->setIcon(QIcon(":/images/search_icon.png"));
}

void LogSearchWidget::createConnections()
{
    connect(m_actExec, &QAction::triggered, this, &LogSearchWidget::startSearch);
    connect(m_actClear, &QAction::triggered, this, &LogSearchWidget::clearResults);
    connect(m_actAddSearchParam, &QAction::triggered, this, &LogSearchWidget::addSearchParam);
    connect(m_sourceModel, &BaseLogModel::modelConfigured, this, &LogSearchWidget::sourceModelConfigured);
    connect(m_sourceModel, &BaseLogModel::valueFound, this, &LogSearchWidget::addSearchResult);
    connect(m_searchResults, &LogViewWidget::rowSelected, this, &LogSearchWidget::rowSelected);
}

void LogSearchWidget::addSearchParam()
{
    SearchParamWidget *param = new SearchParamWidget(m_proxyModel, this);
    param->setColumns(m_sourceModel->getColumns());
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

    SearchParamLst params;

    for (auto paramWidget : m_searchParamWidgets)
    {
        if (paramWidget->getIsEnabled())
        {
            SearchParam param;
            param.matchCase = paramWidget->matchCase();
            param.isRegex = paramWidget->isRegex();
            param.wholeText = paramWidget->matchWholeText();
            param.notOp = paramWidget->notOp();
            param.exp = paramWidget->expression();
            param.column = paramWidget->column();
            if (!param.exp.empty())
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

void LogSearchWidget::addSearchResult(std::shared_ptr<std::deque<ssize_t>> rowsPtr)
{
    if (m_sourceModel->isSearching())
    {
        m_proxyModel->addRows(*rowsPtr.get());
        m_searchResults->updateView();
    }
}

void LogSearchWidget::clearResults()
{
    m_sourceModel->stopSearch();
    m_proxyModel->clear();
    m_searchResults->updateView();
}

void LogSearchWidget::deleteParamWidget(QWidget *paramWidget)
{
    for (std::size_t i = 0; i < m_searchParamWidgets.size(); ++i)
    {
        if (m_searchParamWidgets.at(i) == paramWidget)
        {
            qDebug() << "Removing param" << i;
            m_searchParamWidgets.removeAt(i);
            paramWidget->deleteLater();
            return;
        }
    }
}

void LogSearchWidget::sourceModelConfigured()
{
    if (m_searchParamWidgets.empty())
    {
        addSearchParam();
    }
}