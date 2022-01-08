#include "LogTabWidget.h"
#include "LogViewWidget.h"
#include "LogSearchWidget.h"
#include "TextLogModel.h"
#include "JsonLogModel.h"
#include "LongScrollBar.h"
#include <QTableView>
#include <QDebug>
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

LogTabWidget::LogTabWidget(QWidget *parent) : QWidget(parent)
{
    createActions();

    m_logModel = new JsonLogModel("/home/rafael/Dev/QLogViewer/log.json", this);
    m_logViewWidget = new LogViewWidget(this);
    m_logViewWidget->setMinimumSize(400, 200);
    m_logViewWidget->setLogModel(m_logModel);

    m_logSearchWidget = new LogSearchWidget(m_logModel, this);

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
}

void LogTabWidget::createActions()
{

}

void LogTabWidget::createConnections()
{
    connect(m_logSearchWidget, &LogSearchWidget::rowSelected, m_logViewWidget, &LogViewWidget::goToRow);
}
