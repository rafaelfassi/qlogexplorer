// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#include "pch.h"
#include "MainWindow.h"
#include "Settings.h"
#include "LogTabWidget.h"
#include "LogViewWidget.h"
#include "TextLogModel.h"
#include "JsonLogModel.h"
#include "LongScrollBar.h"
#include "LogSearchWidget.h"
#include "HeaderView.h"
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QAction>
#include <QToolBar>
#include <QMenuBar>
#include <QFileDialog>
#include <QInputDialog>
#include <QFileInfo>
#include <QApplication>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QRect rec = QApplication::desktop()->screenGeometry();
    setMinimumSize(std::min(800, rec.width()), std::min(600, rec.height()));
    createActions();
    createMenus();

    m_tabViews = new QTabWidget(this);
    m_tabViews->setTabsClosable(true);
    setCentralWidget(m_tabViews);

    createConnections();

    loadConfig();

    // openFile("/home/rafael/Dev/QLogViewer/log.json", tp::FileType::Json);
    // openFile("/home/rafael/Dev/QLogViewer/log.txt", tp::FileType::Text);
    //  openFile("/home/rafael/Dev/QLogViewer/biglog.txt", FileType::Text);
    //  openFile("/home/rafael/Dev/QLogViewer/biglog.json", FileType::Text);
    //  openFile("/home/rafael/Dev/QLogViewer/log.json", FileType::Text);
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
    m_openFile = new QAction(tr("Open"), this);
    m_openFileAsText = new QAction(tp::toStr(tp::FileType::Text).c_str(), this);
    m_openFileAsJson = new QAction(tp::toStr(tp::FileType::Json).c_str(), this);
    m_actSaveConf = new QAction(tr("Save Configuration"), this);
    m_actSaveConf->setVisible(false);
    m_actSaveConfAs = new QAction(tr("Save As..."), this);
    m_actSaveConfAs->setVisible(false);
    m_actEdtRegex = new QAction(tr("Regular Expression"), this);
    m_actEdtRegex->setEnabled(false);
}

void MainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_openFile);

    m_fileOpenAsMenu = m_fileMenu->addMenu(tr("Open As..."));
    m_fileOpenAsMenu->addAction(m_openFileAsText);
    m_fileOpenAsMenu->addAction(m_openFileAsJson);

    m_fileMenu->addSeparator();

    m_fileMenu->addAction(m_actSaveConf);
    m_fileMenu->addAction(m_actSaveConfAs);

    m_fileMenu->addSeparator();

    m_fileMenu->addAction(m_actEdtRegex);

    m_actRecentFilesSep = m_fileMenu->addSeparator();
}

void MainWindow::createToolBars()
{
    // auto fileToolBar = addToolBar(tr("&File"));
    // fileToolBar->addAction(m_toggeFollowing);
}

void MainWindow::createConnections()
{
    connect(m_openFile, &QAction::triggered, this, [this]() { openFile(tp::FileType::Text); });
    connect(m_openFileAsText, &QAction::triggered, this, [this]() { openFile(tp::FileType::Text); });
    connect(m_openFileAsJson, &QAction::triggered, this, [this]() { openFile(tp::FileType::Json); });
    connect(m_actSaveConf, &QAction::triggered, this, &MainWindow::saveConf);
    connect(m_actSaveConfAs, &QAction::triggered, this, &MainWindow::saveConfAs);
    connect(m_actEdtRegex, &QAction::triggered, this, &MainWindow::editRegex);
    connect(m_tabViews, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    connect(m_tabViews, &QTabWidget::currentChanged, this, &MainWindow::confCurrentTab);
}

void MainWindow::loadConfig()
{
    updateTemplates();
    updateRecentFiles();
}

void MainWindow::updateTemplates()
{
    for (auto &actTempl : m_actTemplates)
    {
        delete actTempl.first;
    }
    m_actTemplates.clear();

    const auto &templates = Settings::getTemplates();
    int idx(0);
    for (auto conf : templates)
    {
        QAction *act = new QAction(conf->getConfigName().c_str(), this);
        act->setData(idx++);
        connect(act, &QAction::triggered, this, &MainWindow::handleOpenWithTemplate);
        m_actTemplates.emplace_back(std::make_pair(act, conf));
        m_fileOpenAsMenu->addAction(act);
    }
}

void MainWindow::handleOpenWithTemplate()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action == nullptr || action->data().isNull())
        return;

    auto idx = action->data().toInt();
    if (idx >= 0 && idx < m_actTemplates.size())
    {
        Conf *conf = m_actTemplates[idx].second;
        const auto fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
        if (fileName.isEmpty())
            return;
        auto newConf = new Conf(*conf);
        newConf->setFileName(fileName.toStdString());
        openFile(newConf);
    }
}

void MainWindow::setRecentFile(Conf *conf)
{
    Settings::setRecentFile(conf);
    updateRecentFiles();
}

void MainWindow::handleOpenRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action == nullptr || action->data().isNull())
        return;

    const auto idx = action->data().toInt();
    if (idx >= 0 && idx < m_actRecentFiles.size())
    {
        const auto &recentFileConf = m_actRecentFiles[idx].second;
        Conf *conf = new Conf(recentFileConf);
        openFile(conf);
    }
}

void MainWindow::updateRecentFiles()
{
    const auto &recentFiles = Settings::getRecentFiles();

    for (auto &recentFile : m_actRecentFiles)
    {
        delete recentFile.first;
    }
    m_actRecentFiles.clear();

    int idx(0);
    for (const auto &conf : recentFiles)
    {
        std::string typeStr = conf.getConfigName();
        if (typeStr.empty())
        {
            typeStr = tp::toStr<tp::FileType>(conf.getFileType());
        }
        const auto dispName = QString("%1 As [%2]").arg(utl::elideLeft(conf.getFileName(), 60), typeStr.c_str());
        auto act = new QAction(dispName, this);
        act->setData(idx++);
        connect(act, &QAction::triggered, this, &MainWindow::handleOpenRecentFile);
        m_actRecentFiles.emplace_back(std::make_pair(act, conf));
        m_fileMenu->addAction(act);
    }

    m_actRecentFilesSep->setVisible(!recentFiles.empty());
}

void MainWindow::openFile(tp::FileType type)
{
    const auto fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
    if (fileName.isEmpty())
    {
        return;
    }

    openFile(fileName, type);
}

void MainWindow::openFile(const QString &fileName, tp::FileType type)
{
    Conf *conf = new Conf(type);
    conf->setFileName(fileName.toStdString());
    // if (type == tp::FileType::Text)
    // {
    //     //conf->setRegexPattern("^\\[(?<Level>[A-Z])\\]:\\s+(?<Time>\\d{2}-\\d{2}-\\d{4}\\s+\\d{2}:\\d{2}:\\d{2}\\.\\d+)\\s+\\[(?<File>[^\\]]+)\\]:\\s+(?<Message>.*)$");
    //     //conf->setRegexPattern("^\\[([A-Z])\\]:\\s+(\\d{2}-\\d{2}-\\d{4}\\s+\\d{2}:\\d{2}:\\d{2}\\.\\d+)\\s+\\[([^\\]]+)\\]:\\s+(.*)$");
    // }
    openFile(conf);
}

void MainWindow::openFile(Conf *conf)
{
    QFileInfo fileInfo(conf->getFileName().c_str());
    if (!fileInfo.exists() || !fileInfo.isFile())
    {
        return;
    }

    conf->setFileName(fileInfo.filePath().toStdString());
    LogTabWidget *logTabWidget = new LogTabWidget(conf, this);

    int newTabIdx = m_tabViews->addTab(logTabWidget, fileInfo.fileName());
    m_tabViews->setCurrentIndex(newTabIdx);
    setRecentFile(conf);
}

void MainWindow::editRegex()
{
    LogTabWidget *tab = qobject_cast<LogTabWidget *>(m_tabViews->currentWidget());
    if (!tab)
    {
        LOG_ERR("No current tab");
        return;
    }

    bool ok;
    QString rxPattern = QInputDialog::getText(
        this,
        tr("Regular Expression"),
        tr("Pattern:"),
        QLineEdit::Normal,
        tab->getConf().getRegexPattern().c_str(),
        &ok);

    if (ok)
    {
        tab->getConf().setRegexPattern(rxPattern.toStdString());
        tab->updateColumns();
    }
}

void MainWindow::closeTab(int index)
{
    LogTabWidget *tab = qobject_cast<LogTabWidget *>(m_tabViews->widget(index));
    if (!tab)
    {
        LOG_ERR("No tab for index {}", index);
        return;
    }

    m_tabViews->removeTab(index);
    delete tab;
}

void MainWindow::confCurrentTab(int index)
{
    if (index < 0)
    {
        m_actSaveConf->setVisible(false);
        m_actSaveConfAs->setVisible(false);
        m_actEdtRegex->setEnabled(false);
        return;
    }

    LogTabWidget *tab = qobject_cast<LogTabWidget *>(m_tabViews->widget(index));
    if (!tab)
    {
        LOG_ERR("No current tab for index {}", index);
        return;
    }

    Conf &conf = tab->getConf();
    m_actSaveConf->setVisible(true);
    if (conf.exists())
    {
        m_actSaveConf->setText(QString("Save [%1]").arg(conf.getConfigName().c_str()));
        m_actSaveConfAs->setVisible(true);
        m_actSaveConfAs->setText(QString("Save [%1] As...").arg(conf.getConfigName().c_str()));
    }
    else
    {
        m_actSaveConf->setText("Save Configuration");
        m_actSaveConfAs->setVisible(false);
        m_actSaveConfAs->setText("Save Configuration As...");
    }

    m_actEdtRegex->setEnabled(conf.getFileType() == tp::FileType::Text);
}

void MainWindow::saveConf()
{
    LogTabWidget *tab = qobject_cast<LogTabWidget *>(m_tabViews->currentWidget());
    if (!tab)
    {
        LOG_ERR("No current tab");
        return;
    }

    auto &conf = tab->getConf();
    if (conf.exists())
    {
        Settings::saveTemplate(&conf);
    }
    else
    {
        saveConfAs();
    }
}

void MainWindow::saveConfAs()
{
    LogTabWidget *tab = qobject_cast<LogTabWidget *>(m_tabViews->currentWidget());
    if (!tab)
    {
        LOG_ERR("No current tab");
        return;
    }

    auto &conf = tab->getConf();
    bool ok;
    QString confName =
        QInputDialog::getText(this, tr("Save template"), tr("Template name:"), QLineEdit::Normal, QString(), &ok);
    if (ok && !confName.isEmpty())
    {
        Settings::saveTemplateAs(&conf, confName);
        updateTemplates();
        setRecentFile(&conf);
        const auto idx = m_tabViews->currentIndex();
        if (idx >= 0)
        {
            confCurrentTab(0);
        }
    }
}