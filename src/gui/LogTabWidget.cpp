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

LogTabWidget::LogTabWidget(const QString &fileName, FileType type, QWidget *parent) : QWidget(parent)
{
    createActions();

    switch (type)
    {
        case FileType::Text:
            m_logModel = new TextLogModel(fileName.toStdString(), this);
            break;
        case FileType::Json:
            m_logModel = new JsonLogModel(fileName.toStdString(), this);
            break;
    }

    m_logViewWidget = new LogViewWidget(m_logModel, this);
    m_logViewWidget->setMinimumSize(400, 200);

    m_logSearchWidget = new LogSearchWidget(m_logViewWidget, m_logModel, this);

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

}
