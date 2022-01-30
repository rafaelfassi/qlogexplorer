#include "pch.h"
#include "MainWindow.h"
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
#include <QStandardPaths>
#include <QSettings>

#include <fstream>

constexpr int g_maxRecentFiles(10);

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
    m_actSaveConf->setEnabled(false);
    m_actSaveConfAs = new QAction(tr("Save Configuration As..."), this);
    m_actSaveConfAs->setEnabled(false);
    m_actEdtRegex = new QAction(tr("Regular Expression"), this);
    m_actEdtRegex->setEnabled(false);

    m_actRecentFiles.resize(g_maxRecentFiles);
    for (int i = 0; i < g_maxRecentFiles; ++i)
    {
        m_actRecentFiles[i] = new QAction(this);
        m_actRecentFiles[i]->setVisible(false);
    }
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
    for (int i = 0; i < g_maxRecentFiles; ++i)
    {
        m_fileMenu->addAction(m_actRecentFiles[i]);
    }
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

    for (int i = 0; i < g_maxRecentFiles; ++i)
    {
        connect(m_actRecentFiles[i], &QAction::triggered, this, &MainWindow::handleOpenRecentFile);
    }
}

void MainWindow::loadConfig()
{
    const auto &confPaths = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    if (confPaths.isEmpty())
    {
        LOG_ERR("Cannot locate the default configuration directory");
        return;
    }

    m_configDir.setPath(confPaths.first());
    if (!m_configDir.exists())
    {
        if (!m_configDir.mkpath(m_configDir.absolutePath()))
        {
            LOG_ERR("Cannot create config directory {}", utl::toStr(m_configDir.absolutePath()));
            return;
        }
    }

    m_templatesDir.setPath(m_configDir.absolutePath() + QDir::separator() + "templates");
    if (!m_templatesDir.exists())
    {
        if (!m_templatesDir.mkpath(m_templatesDir.absolutePath()))
        {
            LOG_ERR("Cannot create templates directory {}", utl::toStr(m_configDir.absolutePath()));
            return;
        }
    }

    const auto &settingsFile = m_configDir.absoluteFilePath("settings.ini");
    m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);

    updateTemplates();
    updateRecentFiles();
}

void MainWindow::updateTemplates()
{
    const auto &templates = m_templatesDir.entryList(QDir::Files | QDir::Readable);
    if (!templates.isEmpty())
    {
        if (m_templates.empty())
            m_fileOpenAsMenu->addSeparator();

        for (const auto &templ : templates)
        {
            const auto &templFileName = m_templatesDir.absoluteFilePath(templ).toStdString();
            if (findConfByTemplateFileName(templFileName) == nullptr)
            {
                Conf *conf = new Conf(templFileName);
                const int idx = m_templates.size();
                m_templates.push_back(conf);
                QAction *actOpenTempl = new QAction(conf->getConfigName().c_str(), this);
                actOpenTempl->setData(idx);
                connect(actOpenTempl, &QAction::triggered, this, &MainWindow::handleOpenWithTemplate);
                m_fileOpenAsMenu->addAction(actOpenTempl);
            }
        }
    }
}

void MainWindow::handleOpenWithTemplate()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action == nullptr || action->data().isNull())
        return;

    auto confIdx = action->data().toInt();
    if (confIdx >= 0 && confIdx < m_templates.size())
    {
        Conf *conf = m_templates[confIdx];
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
    QStringList files = m_settings->value("recentFileList").toStringList();
    const auto &fileType = tp::toStr<tp::FileType>(conf->getFileType());
    const auto &fileName = conf->getFileName();
    const auto &templateFileName = conf->getConfFileName();

    const QString recentFile = utl::join({fileType, fileName, templateFileName}, "|").c_str();

    files.removeAll(recentFile);
    files.prepend(recentFile);
    while (files.size() > g_maxRecentFiles)
    {
        files.removeLast();
    }

    m_settings->setValue("recentFileList", files);

    updateRecentFiles();
}

void MainWindow::handleOpenRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        const auto recentFile = utl::split(action->data().toString().toStdString(), "|");
        if (recentFile.size() != 3)
        {
            LOG_ERR("Invalid refent file format");
            return;
        }
        const auto &fileType = tp::fromStr<tp::FileType>(recentFile[0]);
        const auto &fileName = recentFile[1];
        const auto &templateFileName = recentFile[2];
        if (templateFileName.empty())
        {
            openFile(fileName.c_str(), fileType);
        }
        else
        {
            const auto templCong = findConfByTemplateFileName(templateFileName);
            if (templCong == nullptr)
            {
                LOG_ERR("Template '{}' not found", templateFileName);
                return;
            }

            Conf *conf = new Conf(*templCong);
            conf->setFileName(fileName);
            openFile(conf);
        }
    }
}

Conf *MainWindow::findConfByTemplateFileName(const std::string &templateFileName)
{
    const auto it = std::find_if(
        m_templates.begin(),
        m_templates.end(),
        [&templateFileName](const Conf *conf) { return (conf->getConfFileName() == templateFileName); });

    if (it != m_templates.end())
    {
        return *it;
    }
    return nullptr;
}

void MainWindow::updateRecentFiles()
{
    QStringList files = m_settings->value("recentFileList").toStringList();

    int numRecentFiles = std::min(files.size(), g_maxRecentFiles);

    for (int i = 0, actIdx = 0; i < numRecentFiles; ++i, ++actIdx)
    {
        QString text = makeRecentFileName(files[i].toStdString());
        if (text.isEmpty())
        {
            --actIdx;
            continue;
        }
        m_actRecentFiles[actIdx]->setText(text);
        m_actRecentFiles[actIdx]->setData(files[i]);
        m_actRecentFiles[actIdx]->setVisible(true);
    }

    for (int j = numRecentFiles; j < g_maxRecentFiles; ++j)
    {
        m_actRecentFiles[j]->setVisible(false);
    }

    m_actRecentFilesSep->setVisible(numRecentFiles > 0);
}

QString MainWindow::makeRecentFileName(const std::string &recentFile)
{
    const auto &recentFileParts = utl::split(recentFile, "|");
    const auto &fileType = recentFileParts[0];
    const auto &fileName = recentFileParts[1];
    const auto &templFileName = recentFileParts[2];

    QString typeStr;
    if (!templFileName.empty())
    {
        const auto templCong = findConfByTemplateFileName(templFileName);
        if (templCong != nullptr)
            typeStr = templCong->getConfigName().c_str();
        else
            return QString(); // Template was deleted.
    }
    else
    {
        typeStr = fileType.c_str();
    }

    return QString("%1 As [%2]").arg(utl::elideLeft(fileName, 60), typeStr);
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
        m_actSaveConf->setEnabled(false);
        m_actSaveConfAs->setEnabled(false);
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
    m_actSaveConf->setEnabled(true);
    m_actSaveConfAs->setEnabled(conf.exists());
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
        conf.saveConf();
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
        conf.setConfigName(confName.toStdString());

        QString fileName = confName.simplified().toLower().replace(QRegularExpression("[^a-z\\d-]"), "_");
        if (fileName.isEmpty())
        {
            fileName = "template";
        }
        int cnt(0);
        QString ext(".json");
        while (m_templatesDir.exists(fileName + ext))
        {
            fileName.append(QString("_%1").arg(++cnt));
        }
        fileName.append(ext);

        fileName = m_templatesDir.absoluteFilePath(fileName);
        conf.saveConfAs(fileName.toStdString());

        updateTemplates();
        setRecentFile(&conf);
    }
}