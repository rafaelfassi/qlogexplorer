// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "LogTabWidget.h"
#include "LogViewWidget.h"
#include "LogSearchWidget.h"
#include "Style.h"
#include "TextLogModel.h"
#include "JsonLogModel.h"
#include "LongScrollBar.h"
#include "ProgressLabel.h"
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

LogTabWidget::LogTabWidget(FileConf::Ptr conf, QWidget *parent) : QWidget(parent), m_conf(conf)
{
    if (!conf)
    {
        LOG_ERR("File conf is null");
        return;
    }

    createActions();

    switch (conf->getFileType())
    {
        case tp::FileType::Text:
            m_logModel = new TextLogModel(conf, this);
            break;
        case tp::FileType::Json:
            m_logModel = new JsonLogModel(conf, this);
            break;
        default:
            LOG_ERR("Invalid FileType {}", tp::toSInt(conf->getFileType()));
            m_logModel = new TextLogModel(conf, this);
    }

    QToolBar *toolbar = new QToolBar(tr("File ToolBar"), this);
    toolbar->addAction(m_actTrackFile);
    toolbar->addAction(m_actAutoScrolling);
    m_prlFileParsing = new ProgressLabel(this);
    m_prlFileParsing->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    toolbar->addWidget(m_prlFileParsing);

    m_logViewWidget = new LogViewWidget(m_logModel, m_markedTexts, this);
    m_logViewWidget->setMinimumSize(400, 200);
    m_logViewWidget->configure(conf);

    m_logSearchWidget = new LogSearchWidget(conf, m_logViewWidget, m_logModel, this);
    m_logSearchWidget->configure();

    QSplitter *splitter = new QSplitter(parent);
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(m_logViewWidget);
    splitter->addWidget(m_logSearchWidget);

    splitter->setStretchFactor(0, 4);
    splitter->setStretchFactor(1, 1);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(toolbar);
    vLayout->addWidget(splitter);

    translateUi();
    updateConfName();

    setLayout(vLayout);

    createConnections();
    m_logModel->start();
}

LogTabWidget::~LogTabWidget()
{
}

void LogTabWidget::createActions()
{
    m_actTrackFile = new QAction(this);
    m_actTrackFile->setCheckable(true);
    m_actTrackFile->setChecked(true);

    m_actAutoScrolling = new QAction(this);
    m_actAutoScrolling->setCheckable(true);
    m_actAutoScrolling->setChecked(false);
}

void LogTabWidget::createConnections()
{
    connect(m_actTrackFile, &QAction::toggled, m_logModel, &BaseLogModel::setFollowing);
    connect(m_actAutoScrolling, &QAction::toggled, m_logViewWidget, &LogViewWidget::setAutoScrolling);
    connect(m_logViewWidget, &LogViewWidget::autoScrollingChanged, m_actAutoScrolling, &QAction::setChecked);
    connect(m_logModel, &BaseLogModel::parsingProgressChanged, m_prlFileParsing, &ProgressLabel::setProgress);
}

void LogTabWidget::translateUi()
{
    m_actTrackFile->setText(tr("Track File"));
    m_actTrackFile->setIcon(Style::getIcon("track_icon.png"));

    m_actAutoScrolling->setText(tr("Auto Scrolling"));
    m_actAutoScrolling->setIcon(Style::getIcon("scroll_down.png"));

    m_prlFileParsing->setActionText(tr("Indexing"));
}

void LogTabWidget::retranslateUi()
{
    translateUi();
    Style::updateWidget(m_prlFileParsing);
    m_logViewWidget->retranslateUi();
    m_logSearchWidget->retranslateUi();
}

void LogTabWidget::reconfigure()
{
    m_logViewWidget->reconfigure(m_conf);
    m_logSearchWidget->reconfigure();
    m_logModel->reconfigure();
}

void LogTabWidget::updateConfName()
{
    const QString typeStr = m_conf->getTemplateNameOrType().c_str();
    m_prlFileParsing->setText(QString("%1 [%2]").arg(m_conf->getFileName().c_str()).arg(typeStr));
}

FileConf::Ptr LogTabWidget::getConf()
{
    return m_conf;
}
