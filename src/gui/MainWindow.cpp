// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "MainWindow.h"
#include "Settings.h"
#include "Style.h"
#include "LogTabWidget.h"
#include "LogViewWidget.h"
#include "TextLogModel.h"
#include "JsonLogModel.h"
#include "LongScrollBar.h"
#include "LogSearchWidget.h"
#include "HeaderView.h"
#include "TemplatesConfigDlg.h"
#include "SettingsDlg.h"
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
#include <QMessageBox>
#include <QDesktopServices>
#include <QScreen>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(APP_NAME);
    setWindowIcon(QIcon(":/images/qlogexplorer.png"));
    QRect rec = QGuiApplication::primaryScreen()->geometry();
    setMinimumSize(std::min(800, rec.width()), std::min(600, rec.height()));
    createActions();
    createMenus();

    m_tabViews = new QTabWidget(this);
    m_tabViews->setTabsClosable(true);
    setCentralWidget(m_tabViews);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(1000);

    translateUi();

    createConnections();

    updateMenus();

    setAcceptDrops(true);

    TemplatesConfigDlg::setMainWindow(this);

    configAsCurrentTab(-1);
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
    m_actOpenFileAsText = new QAction(tp::toStr(tp::FileType::Text).c_str(), this);
    m_actOpenFileAsJson = new QAction(tp::toStr(tp::FileType::Json).c_str(), this);

    m_actReopenFileAsText = new QAction(tp::toStr(tp::FileType::Text).c_str(), this);
    m_actReopenFileAsJson = new QAction(tp::toStr(tp::FileType::Json).c_str(), this);

    m_actCloseFile = new QAction(this);
    m_actCloseFile->setShortcut(QKeySequence::Close);

    m_actQuit = new QAction(this);
    m_actQuit->setShortcut(QKeySequence::Quit);

    m_actSaveConf = new QAction(this);
    m_actSaveConf->setVisible(false);

    m_actSaveConfAs = new QAction(this);
    m_actSaveConfAs->setVisible(false);

    m_actTemplatesConfig = new QAction(this);

    m_actSettings = new QAction(this);

    m_actOpenWiki = new QAction(this);
    m_actAbout = new QAction(this);
}

void MainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu("File");

    m_fileOpenAsMenu = m_fileMenu->addMenu("Open As...");
    m_fileOpenAsMenu->addAction(m_actOpenFileAsText);
    m_fileOpenAsMenu->addAction(m_actOpenFileAsJson);
    m_actOpenAsSep = m_fileOpenAsMenu->addSeparator();

    m_fileOpenRecent = m_fileMenu->addMenu("Open Recent");

    m_fileMenu->addSeparator();

    m_fileReopenAsMenu = m_fileMenu->addMenu("Reopen As...");
    m_fileReopenAsMenu->addAction(m_actReopenFileAsText);
    m_fileReopenAsMenu->addAction(m_actReopenFileAsJson);
    m_fileReopenAsMenu->addAction(m_actOpenAsSep);

    m_fileMenu->addSeparator();

    m_fileMenu->addAction(m_actCloseFile);

    m_fileMenu->addSeparator();

    m_fileMenu->addAction(m_actQuit);

    m_templatesMenu = menuBar()->addMenu("Templates");

    m_templatesMenu->addAction(m_actSaveConf);
    m_templatesMenu->addAction(m_actSaveConfAs);
    m_templatesMenu->addSeparator();
    m_templatesMenu->addAction(m_actTemplatesConfig);

    m_toolsMenu = menuBar()->addMenu("Utilities");
    m_toolsMenu->addAction(m_actSettings);

    m_helpMenu = menuBar()->addMenu("Help");
    m_helpMenu->addAction(m_actOpenWiki);
    m_helpMenu->addAction(m_actAbout);
}

void MainWindow::createToolBars()
{
}

void MainWindow::createConnections()
{
    connect(m_actOpenFileAsText, &QAction::triggered, this, [this]() { openFiles(tp::FileType::Text); });
    connect(m_actOpenFileAsJson, &QAction::triggered, this, [this]() { openFiles(tp::FileType::Json); });

    connect(m_actReopenFileAsText, &QAction::triggered, this, [this]() { reopenCurrentFile(tp::FileType::Text); });
    connect(m_actReopenFileAsJson, &QAction::triggered, this, [this]() { reopenCurrentFile(tp::FileType::Json); });

    connect(m_actCloseFile, &QAction::triggered, this, &MainWindow::closeCurrentTab);

    connect(m_actQuit, &QAction::triggered, qApp, &QApplication::quit);

    connect(m_actSaveConf, &QAction::triggered, this, &MainWindow::saveConf);
    connect(m_actSaveConfAs, &QAction::triggered, this, &MainWindow::saveConfAs);
    connect(m_actTemplatesConfig, &QAction::triggered, this, &MainWindow::openTemplatesConfig);
    connect(m_actSettings, &QAction::triggered, this, &MainWindow::openSettings);
    connect(m_actOpenWiki, &QAction::triggered, this, &MainWindow::openWiki);
    connect(m_actAbout, &QAction::triggered, this, &MainWindow::openAbout);
    connect(m_tabViews, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    connect(m_tabViews, &QTabWidget::currentChanged, this, &MainWindow::configAsCurrentTab);
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateCurrentTab);
}

void MainWindow::translateUi()
{
    m_fileMenu->setTitle(tr("&File"));
    m_fileOpenAsMenu->setTitle(tr("&Open As..."));
    m_fileOpenRecent->setTitle(tr("Open &Recent"));
    m_fileReopenAsMenu->setTitle(tr("Reopen &As..."));
    m_actCloseFile->setText(tr("&Close File"));
    m_actQuit->setText(tr("&Quit"));

    m_templatesMenu->setTitle(tr("&Templates"));
    m_actTemplatesConfig->setText(tr("&Configure Templates"));

    m_toolsMenu->setTitle(tr("&Utilities"));
    m_actSettings->setText(tr("&Settings"));

    m_helpMenu->setTitle(tr("&Help"));
    m_actOpenWiki->setText(tr("&Wiki"));
    m_actAbout->setText(tr("&About"));
}

void MainWindow::retranslateUi()
{
    translateUi();

    Style::updateWidget(this);
    Style::updateWidget(menuBar());
    Style::updateWidget(m_tabViews->tabBar());
    for (int tabIdx = 0; tabIdx < getTabsCount(); ++tabIdx)
    {
        auto tab = getTab(tabIdx);
        if (tab)
        {
            tab->retranslateUi();
        }
    }
}

void MainWindow::reconfigure()
{
    for (int tabIdx = 0; tabIdx < getTabsCount(); ++tabIdx)
    {
        auto tab = getTab(tabIdx);
        if (tab)
        {
            tab->reconfigure();
        }
    }
}

void MainWindow::updateCurrentTabConf(FileConf::Ptr conf)
{
    if (!conf || conf->isNull())
        return;

    auto currTab = getCurrentTab();
    if (currTab)
    {
        auto currTabConf = currTab->getConf();
        if (currTabConf)
        {
            currTabConf->copyFrom(conf);
            currTab->reconfigure();
        }
    }
}

void MainWindow::updateOpenedConf(FileConf::Ptr conf)
{
    if (!conf || conf->isNull())
        return;

    for (int tabIdx = 0; tabIdx < getTabsCount(); ++tabIdx)
    {
        auto tab = getTab(tabIdx);
        if (!tab)
            continue;

        auto tabConf = tab->getConf();
        if (tabConf->exists() && conf->isSameType(tabConf) && conf->isDirt(tabConf))
        {
            const auto file = tabConf->getFileName();
            tabConf->copyFrom(conf);
            tabConf->setFileName(file);
            tab->reconfigure();
        }
    }
}

void MainWindow::updateAllOpenedConfs()
{
    const auto templates = Settings::getTemplates();
    for (auto &templ : templates)
    {
        updateOpenedConf(templ);
    }
}

void MainWindow::updateMenus()
{
    updateTemplates();
    updateRecentFiles();
}

int MainWindow::getTabsCount() const
{
    return m_tabViews->count();
}

int MainWindow::getCurrentTabIdx() const
{
    return m_tabViews->currentIndex();
}

LogTabWidget *MainWindow::getCurrentTab() const
{
    return getTab(getCurrentTabIdx());
}

LogTabWidget *MainWindow::getTab(int idx) const
{
    if ((-1 < idx) && (idx < m_tabViews->count()))
        return qobject_cast<LogTabWidget *>(m_tabViews->widget(idx));
    return nullptr;
}

void MainWindow::setFilesToOpen(const QStringList &files)
{
    m_filesToOpen = files;
}

void MainWindow::clearFilesToOpen()
{
    m_filesToOpen.clear();
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
    for (auto &act : m_actOpenWithTemplates)
    {
        delete act.first;
    }
    m_actOpenWithTemplates.clear();

    for (auto &act : m_actReopenWithTemplates)
    {
        delete act.first;
    }
    m_actReopenWithTemplates.clear();

    int idx(0);
    const auto &templates = Settings::getTemplates();
    for (auto conf : templates)
    {
        const QString name(conf->getConfigName().c_str());

        {
            QAction *actOpen = new QAction(name, this);
            actOpen->setData(idx);
            connect(actOpen, &QAction::triggered, this, &MainWindow::handleOpenFilesWithTemplate);
            m_actOpenWithTemplates.emplace_back(std::make_pair(actOpen, conf));
            m_fileOpenAsMenu->addAction(actOpen);
        }

        {
            QAction *actReopen = new QAction(name, this);
            actReopen->setData(idx);
            connect(actReopen, &QAction::triggered, this, &MainWindow::handleReopenWithTemplate);
            m_actReopenWithTemplates.emplace_back(std::make_pair(actReopen, conf));
            m_fileReopenAsMenu->addAction(actReopen);
        }

        idx++;
    }

    m_actOpenAsSep->setVisible(!templates.empty());
}

void MainWindow::handleOpenFilesWithTemplate()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action == nullptr || action->data().isNull())
        return;

    auto idx = action->data().toInt();
    if (idx >= 0 && idx < m_actOpenWithTemplates.size())
    {
        auto conf = m_actOpenWithTemplates[idx].second;
        const auto &files = getFilesToOpen();
        for (const auto &fileName : files)
        {
            auto newConf = FileConf::clone(conf);
            newConf->setFileName(fileName.toStdString());
            openFile(newConf);
        }
    }

    clearFilesToOpen();
}

void MainWindow::handleReopenWithTemplate()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action == nullptr || action->data().isNull())
        return;

    auto idx = action->data().toInt();
    if (idx >= 0 && idx < m_actReopenWithTemplates.size())
    {
        auto conf = m_actReopenWithTemplates[idx].second;
        reopenCurrentFile(conf);
    }
}

void MainWindow::setRecentFile(const FileConf::Ptr &conf)
{
    Settings::setRecentFile(conf);
    updateRecentFiles();
}

void MainWindow::openWiki()
{
    QDesktopServices::openUrl(QUrl(WIKI_URL, QUrl::TolerantMode));
}

void MainWindow::openAbout()
{
    const QString copyright(R"__(&copy; 2022 Rafael Fassi Lobao)__");
    const QString license(R"__(<a href="https://www.gnu.org/licenses/gpl-3.0.html">GPL-3.0</a>)__");
    const QString project(QString(R"__(<a href="%1">%2</a>)__").arg(PROJ_URL, tr("project")));

    // clang-format off
    QMessageBox::about(
        this,
        tr("About %1").arg(APP_NAME),
        QString("<h2>%1 %2</h2>").arg(APP_NAME, APP_VERSION)
        + "<p>" + tr("Advanced tool for exploring log files") + "</p>"
        + "<p>" + tr("Copyright %1").arg(copyright) + "</p>"
        + "<p>" + tr("This software is licensed under %1").arg(license) + "</p>"
        + "<p>" + tr("Using Qt version %1").arg(QT_VERSION_STR) + "</p>"
        + "<p>" + tr("Visit the %1").arg(project) + "</p>"
        + QString(R"__(
            <p>
                <b>%1</b>
                <ul>
                    <li><a href="https://www.qt.io">Qt</a></li>
                    <li><a href="https://github.com/fmtlib/fmt">fmt</a></li>
                    <li><a href="https://rapidjson.org">RapidJSON</a></li>
                </ul>
            </p>
        )__").arg(tr("Credits")));
    // clang-format on
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
        const QString typeStr = conf->getTemplateNameOrType().c_str();
        const auto dispName = tr("%1 As [%2]").arg(utl::elideLeft(conf->getFileName(), 60), typeStr);
        auto act = new QAction(dispName, this);
        act->setData(idx++);
        connect(act, &QAction::triggered, this, &MainWindow::handleOpenRecentFile);
        m_actRecentFiles.emplace_back(std::make_pair(act, conf));
        m_fileOpenRecent->addAction(act);
    }
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

bool MainWindow::hasOpenedType(const FileConf::Ptr &conf)
{
    for (int tabIdx = 0; tabIdx < getTabsCount(); ++tabIdx)
    {
        auto tab = getTab(tabIdx);
        if (!tab)
            continue;
        const auto tabConf = tab->getConf();
        if (tabConf->isSameType(conf))
            return true;
    }

    return false;
}

void MainWindow::openFile(FileConf::Ptr conf, int idx)
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

    int newTabIdx(-1);
    if (idx == -1)
        newTabIdx = m_tabViews->addTab(logTabWidget, fileInfo.fileName());
    else
        newTabIdx = m_tabViews->insertTab(idx, logTabWidget, fileInfo.fileName());

    goToTab(newTabIdx);
    setRecentFile(conf);
}

void MainWindow::openFile(const QString &fileName, tp::FileType type)
{
    auto conf = FileConf::make(type);
    conf->setFileName(fileName.toStdString());
    openFile(conf);
}

void MainWindow::openFiles(tp::FileType type)
{
    const auto &files = getFilesToOpen();
    for (const auto &fileName : files)
    {
        openFile(fileName, type);
    }
    clearFilesToOpen();
}

void MainWindow::openFiles(const QString &typeOrTemplateName)
{
    const auto &name = utl::toStr(typeOrTemplateName);

    if (auto type = tp::fromStr<tp::FileType>(name); type != tp::FileType::None)
    {
        openFiles(type);
    }
    else if (auto conf = Settings::findConfByTemplateName(name); conf)
    {
        const auto &files = getFilesToOpen();
        for (const auto &fileName : files)
        {
            auto newConf = FileConf::clone(conf);
            newConf->setFileName(fileName.toStdString());
            openFile(newConf);
        }
    }
    else
    {
        LOG_ERR("Could not find any type or template that matches '{}'", name);
    }

    clearFilesToOpen();
}

void MainWindow::reopenCurrentFile(FileConf::Ptr conf)
{
    auto tabIdx = getCurrentTabIdx();
    auto tab = getTab(tabIdx);
    if (tab == nullptr)
        return;

    auto newConf = FileConf::clone(conf);
    newConf->setFileName(tab->getConf()->getFileName());
    closeTab(tabIdx);
    openFile(newConf, tabIdx);
}

void MainWindow::reopenCurrentFile(tp::FileType type)
{
    auto conf = FileConf::make(type);
    reopenCurrentFile(conf);
}

void MainWindow::openTemplatesConfig()
{
    TemplatesConfigDlg templConf;
    templConf.exec();
}

void MainWindow::openSettings()
{
    SettingsDlg settingsDlg(this);
    if (settingsDlg.exec() == QDialog::Accepted)
    {
        retranslateUi();
        reconfigure();
        configAsCurrentTab(getCurrentTabIdx());
    }
}

void MainWindow::closeTab(int index)
{
    auto tab = getTab(index);
    if (!tab)
    {
        LOG_ERR("No tab for index {}", index);
        return;
    }

    m_tabViews->removeTab(index);
    delete tab;
}

void MainWindow::closeCurrentTab()
{
    closeTab(getCurrentTabIdx());
}

void MainWindow::updateCurrentTab()
{
    const auto idx = m_tabViews->currentIndex();
    if (idx >= 0)
    {
        configAsCurrentTab(idx);
    }
}

void MainWindow::configAsCurrentTab(int index)
{
    m_updateTimer->stop();

    const bool hasCurrentTab(index >= 0);
    bool currentHasTemplate(false);
    if (hasCurrentTab)
    {
        LogTabWidget *tab = qobject_cast<LogTabWidget *>(m_tabViews->widget(index));
        if (!tab)
        {
            LOG_ERR("No current tab for index {}", index);
            m_updateTimer->start();
            return;
        }

        tab->updateConfName();

        auto conf = tab->getConf();
        if (conf->exists())
        {
            m_actSaveConf->setText(tr("&Save [%1]").arg(conf->getConfigName().c_str()));
            m_actSaveConf->setVisible(true);
            m_actSaveConfAs->setText(tr("Save [%1] &As...").arg(conf->getConfigName().c_str()));
            m_actSaveConfAs->setVisible(true);
            currentHasTemplate = true;

            auto oriConf = Settings::findConfByTemplateFileName(conf->getConfFileName());
            m_actSaveConf->setEnabled(oriConf && conf->isDirt(oriConf));
        }
    }

    m_fileReopenAsMenu->setEnabled(hasCurrentTab);
    m_actCloseFile->setEnabled(hasCurrentTab);

    if (!currentHasTemplate)
    {
        m_actSaveConf->setVisible(false);
        m_actSaveConf->setText("");
        m_actSaveConfAs->setVisible(false);
        m_actSaveConfAs->setText("");
    }

    auto tabBar = m_tabViews->tabBar();
    if (Settings::getHideUniqueTab())
    {
        const bool showTabBar(m_tabViews->count() > 1);
        if (tabBar->isVisible() != showTabBar)
        {
            tabBar->setVisible(showTabBar);
        }
    }
    else if (!tabBar->isVisible())
    {
        tabBar->setVisible(true);
    }

    m_updateTimer->start();
}

void MainWindow::goToTab(int index)
{
    if (index >= 0 && index < m_tabViews->count())
    {
        m_tabViews->setCurrentIndex(index);
        configAsCurrentTab(index);
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
        updateOpenedConf(conf);
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
            configAsCurrentTab(idx);
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
    QStringList filesToOpen;
    if (mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();
        for (int i = 0; i < urlList.size() && i < 32; ++i)
        {
            filesToOpen.append(urlList.at(i).toLocalFile());
        }
    }

    if (!filesToOpen.empty())
    {
        setFilesToOpen(filesToOpen);
        if (m_fileOpenAsMenu->exec(mapToGlobal(event->pos())) != nullptr)
            event->acceptProposedAction();
        clearFilesToOpen();
    }
}
