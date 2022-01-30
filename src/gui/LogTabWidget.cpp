#include "pch.h"
#include "LogTabWidget.h"
#include "LogViewWidget.h"
#include "LogSearchWidget.h"
#include "TextLogModel.h"
#include "JsonLogModel.h"
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
#include <QSplitter>

#include <fstream>

LogTabWidget::LogTabWidget(Conf *conf, QWidget *parent) : QWidget(parent), m_conf(conf)
{
    createActions();

    switch (conf->getFileType())
    {
        case tp::FileType::Text:
            m_logModel = new TextLogModel(*conf, this);
            break;
        case tp::FileType::Json:
            m_logModel = new JsonLogModel(*conf, this);
            break;
        default:
            LOG_ERR("Invalid FileType {}", tp::toInt(conf->getFileType()));
            m_logModel = new TextLogModel(*conf, this);
    }

    m_logViewWidget = new LogViewWidget(m_logModel, this);
    m_logViewWidget->setMinimumSize(400, 200);
    m_logViewWidget->configure(conf);

    m_logSearchWidget = new LogSearchWidget(m_logViewWidget, m_logModel, this);
    m_logSearchWidget->configure(conf);

    QSplitter *splitter = new QSplitter(parent);
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(m_logViewWidget);
    splitter->addWidget(m_logSearchWidget);

    splitter->setStretchFactor(0, 6);
    splitter->setStretchFactor(1, 1);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(splitter);

    setLayout(vLayout);

    createConnections();
    m_logModel->start();
}

LogTabWidget::~LogTabWidget()
{
    delete m_conf;
}

void LogTabWidget::createActions()
{
}

void LogTabWidget::createConnections()
{
}

void LogTabWidget::updateColumns()
{
    m_conf->clearColumns();
    m_logViewWidget->resetColumns();
    m_logSearchWidget->resetColumns();
    m_logModel->reconfigure();
}

Conf &LogTabWidget::getConf()
{
    return *m_conf;
}