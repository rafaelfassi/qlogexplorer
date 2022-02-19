// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

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
#include "TemplatesConfigDlg.h"
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
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>

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

    setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
    m_openFileAsText = new QAction(tp::toStr(tp::FileType::Text).c_str(), this);
    m_openFileAsJson = new QAction(tp::toStr(tp::FileType::Json).c_str(), this);
    m_actSaveConf = new QAction("", this);
    m_actSaveConf->setVisible(false);
    m_actSaveConfAs = new QAction("", this);
    m_actSaveConfAs->setVisible(false);
    m_actTemplatesConfig = new QAction(tr("Configure Templates"), this);
}

void MainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));

    m_fileOpenAsMenu = m_fileMenu->addMenu(tr("Open As..."));
    m_fileOpenAsMenu->addAction(m_openFileAsText);
    m_fileOpenAsMenu->addAction(m_openFileAsJson);
    m_actOpenAsSep = m_fileOpenAsMenu->addSeparator();

    m_fileMenu->addSeparator();

    m_actRecentFilesSep = m_fileMenu->addSeparator();

    m_templatesMenu = menuBar()->addMenu(tr("&Templates"));

    m_templatesMenu->addAction(m_actSaveConf);
    m_templatesMenu->addAction(m_actSaveConfAs);
    m_templatesMenu->addSeparator();
    m_templatesMenu->addAction(m_actTemplatesConfig);
}

void MainWindow::createToolBars()
{
    // auto fileToolBar = addToolBar(tr("&File"));
    // fileToolBar->addAction(m_toggeFollowing);
}

void MainWindow::createConnections()
{
    connect(m_openFileAsText, &QAction::triggered, this, [this]() { openFile(tp::FileType::Text); });
    connect(m_openFileAsJson, &QAction::triggered, this, [this]() { openFile(tp::FileType::Json); });
    connect(m_actSaveConf, &QAction::triggered, this, &MainWindow::saveConf);
    connect(m_actSaveConfAs, &QAction::triggered, this, &MainWindow::saveConfAs);
    connect(m_actTemplatesConfig, &QAction::triggered, this, &MainWindow::openTemplatesConfig);
    connect(m_tabViews, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    connect(m_tabViews, &QTabWidget::currentChanged, this, &MainWindow::confCurrentTab);
}

void MainWindow::loadConfig()
{
    updateTemplates();
    updateRecentFiles();
}

QStringList MainWindow::getFilesToOpen()
{
    QStringList filesToOpen;

    if (!m_filesToOpen.isEmpty())
    {
        filesToOpen = m_filesToOpen;
    }
    else
    {
        const auto fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
        if (!fileName.isEmpty())
            filesToOpen.append(fileName);
    }

    return filesToOpen;
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

    m_actOpenAsSep->setVisible(!templates.empty());
}

void MainWindow::handleOpenWithTemplate()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action == nullptr || action->data().isNull())
        return;

    auto idx = action->data().toInt();
    if (idx >= 0 && idx < m_actTemplates.size())
    {
        auto conf = m_actTemplates[idx].second;
        const auto &files = getFilesToOpen();
        for (const auto &fileName : files)
        {
            auto newConf = FileConf::clone(conf);
            newConf->setFileName(fileName.toStdString());
            openFile(newConf);
        }
    }
}

void MainWindow::setRecentFile(const FileConf::Ptr &conf)
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

        FileConf::Ptr newFileConf;
        if (!recentFileConf->getConfFileName().empty())
        {
            auto updatedConf = Settings::findConfByTemplateFileName(recentFileConf->getConfFileName());
            if (!updatedConf)
            {
                LOG_ERR("Template file '{}' not found", recentFileConf->getConfFileName());
                return;
            }

            newFileConf = FileConf::clone(updatedConf);
            newFileConf->setFileName(recentFileConf->getFileName());
        }
        else
        {
            newFileConf = FileConf::clone(recentFileConf);
        }

        openFile(newFileConf);
    }
}

void MainWindow::updateRecentFiles()
{
    for (auto &recentFile : m_actRecentFiles)
    {
        delete recentFile.first;
    }
    m_actRecentFiles.clear();

    int idx(0);
    const auto &recentFiles = Settings::getRecentFiles();
    for (const auto &conf : recentFiles)
    {
        std::string typeStr = conf->getConfigName();
        if (typeStr.empty())
        {
            typeStr = tp::toStr<tp::FileType>(conf->getFileType());
        }
        const auto dispName = tr("%1 As [%2]").arg(utl::elideLeft(conf->getFileName(), 60), typeStr.c_str());
        auto act = new QAction(dispName, this);
        act->setData(idx++);
        connect(act, &QAction::triggered, this, &MainWindow::handleOpenRecentFile);
        m_actRecentFiles.emplace_back(std::make_pair(act, conf));
        m_fileMenu->addAction(act);
    }

    m_actRecentFilesSep->setVisible(!recentFiles.empty());
}

int MainWindow::findOpenedFileTab(const FileConf::Ptr &conf)
{
    for (int tabIdx = 0; tabIdx < m_tabViews->count(); ++tabIdx)
    {
        LogTabWidget *tab = qobject_cast<LogTabWidget *>(m_tabViews->widget(tabIdx));
        if (!tab)
        {
            LOG_ERR("Invalid tab at index {}", tabIdx);
            continue;
        }
        const auto tabConf = tab->getConf();
        if (tabConf->isSameFileAndType(conf))
            return tabIdx;
    }

    return -1;
}

void MainWindow::openFile(tp::FileType type)
{
    const auto &files = getFilesToOpen();
    for (const auto &fileName : files)
    {
        openFile(fileName, type);
    }
}

void MainWindow::openFile(const QString &fileName, tp::FileType type)
{
    auto conf = FileConf::make(type);
    conf->setFileName(fileName.toStdString());
    openFile(conf);
}

void MainWindow::openFile(FileConf::Ptr conf)
{
    if (!conf)
    {
        LOG_ERR("File conf is null");
        return;
    }

    QFileInfo fileInfo(conf->getFileName().c_str());
    if (!fileInfo.exists() || !fileInfo.isFile())
    {
        LOG_ERR("File '{}' does not exist", conf->getFileName());
        return;
    }

    const auto openedFileTabIdx = findOpenedFileTab(conf);
    if (openedFileTabIdx != -1)
    {
        goToTab(openedFileTabIdx);
        return;
    }

    conf->setFileName(fileInfo.filePath().toStdString());
    LogTabWidget *logTabWidget = new LogTabWidget(conf, this);

    int newTabIdx = m_tabViews->addTab(logTabWidget, fileInfo.fileName());
    goToTab(newTabIdx);
    setRecentFile(conf);
}

void MainWindow::openTemplatesConfig()
{
    auto currConf = FileConf::make();
    const auto currTabIdx = m_tabViews->currentIndex();
    LogTabWidget *currTab(nullptr);
    if (currTabIdx >= 0)
    {
        currTab = qobject_cast<LogTabWidget *>(m_tabViews->widget(currTabIdx));
        if (currTab)
        {
            currConf = currTab->getConf();
        }
    }

    TemplatesConfigDlg templConf(currConf, this);
    if (templConf.exec() == QDialog::Accepted)
    {
        updateTemplates();
        updateRecentFiles();

        if (currTab && !currConf->isNull())
        {
            currTab->reconfigure();
            confCurrentTab(currTabIdx);
        }

        auto temlates = Settings::getTemplates();
        for (int tabIdx = 0; tabIdx < m_tabViews->count(); ++tabIdx)
        {
            if (tabIdx == currTabIdx)
                continue;

            auto *tab = qobject_cast<LogTabWidget *>(m_tabViews->widget(tabIdx));
            if (!tab)
                continue;

            auto tabConf = tab->getConf();
            if (tabConf->exists())
            {
                for (auto &templ : temlates)
                {
                    if (templ->isSameType(tabConf) && !templ->isEqual(tabConf))
                    {
                        const auto file = tabConf->getFileName();
                        tabConf->copyFrom(templ);
                        tabConf->setFileName(file);
                        tab->reconfigure();
                    }
                }
            }
        }
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
    bool currHasTemplate(false);
    if (index >= 0)
    {
        LogTabWidget *tab = qobject_cast<LogTabWidget *>(m_tabViews->widget(index));
        if (!tab)
        {
            LOG_ERR("No current tab for index {}", index);
            return;
        }

        auto conf = tab->getConf();
        if (conf->exists())
        {
            m_actSaveConf->setText(tr("Save [%1]").arg(conf->getConfigName().c_str()));
            m_actSaveConf->setVisible(true);
            m_actSaveConfAs->setText(tr("Save [%1] As...").arg(conf->getConfigName().c_str()));
            m_actSaveConfAs->setVisible(true);
            currHasTemplate = true;
        }
    }

    if (!currHasTemplate)
    {
        m_actSaveConf->setVisible(false);
        m_actSaveConf->setText("");
        m_actSaveConfAs->setVisible(false);
        m_actSaveConfAs->setText("");
    }

    // Should it hide the tab bar when there's only one file opened?
    // If so, it needs to add a close option in the File menu.
    // const bool showTabBar(m_tabViews->count() > 1);
    // auto tabBar = m_tabViews->tabBar();
    // if (tabBar->isVisible() != showTabBar)
    // {
    //     tabBar->setVisible(showTabBar);
    // }
}

void MainWindow::goToTab(int index)
{
    if (index >= 0 && index < m_tabViews->count())
    {
        m_tabViews->setCurrentIndex(index);
        confCurrentTab(index);
    }
}

void MainWindow::saveConf()
{
    LogTabWidget *tab = qobject_cast<LogTabWidget *>(m_tabViews->currentWidget());
    if (!tab)
    {
        LOG_ERR("No current tab");
        return;
    }

    auto conf = tab->getConf();
    if (conf->exists())
    {
        Settings::saveTemplate(conf);
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

    auto conf = tab->getConf();
    bool ok;
    QString confName =
        QInputDialog::getText(this, tr("Save template"), tr("Template name"), QLineEdit::Normal, QString(), &ok);
    if (ok && !confName.isEmpty())
    {
        Settings::saveTemplateAs(conf, confName);
        updateTemplates();
        setRecentFile(conf);
        const auto idx = m_tabViews->currentIndex();
        if (idx >= 0)
        {
            confCurrentTab(idx);
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();
        for (int i = 0; i < urlList.size() && i < 32; ++i)
        {
            m_filesToOpen.append(urlList.at(i).toLocalFile());
        }
    }

    if (!m_filesToOpen.empty())
    {
        if (m_fileOpenAsMenu->exec(mapToGlobal(event->pos())) != nullptr)
            event->acceptProposedAction();
        m_filesToOpen.clear();
    }
}