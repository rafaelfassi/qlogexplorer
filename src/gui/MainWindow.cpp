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

#include <fstream>

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

    //openFile("/home/rafael/Dev/QLogViewer/log.json", tp::FileType::Json);
    //openFile("/home/rafael/Dev/QLogViewer/log.txt", tp::FileType::Text);
    // openFile("/home/rafael/Dev/QLogViewer/biglog.txt", FileType::Text);
    // openFile("/home/rafael/Dev/QLogViewer/biglog.json", FileType::Text);
    // openFile("/home/rafael/Dev/QLogViewer/log.json", FileType::Text);
}

MainWindow::~MainWindow()
{

}

void MainWindow::createActions()
{
    m_openFile = new QAction(tr("Open"), this);
    m_openFileAsText = new QAction(tr("Plain Text File"), this);
    m_openFileAsJson = new QAction(tr("Json Log File"), this);
    m_actSave = new QAction(tr("Save Template"), this);
}

void MainWindow::createMenus()
{
    auto fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_openFile);

    auto openAsMenu = fileMenu->addMenu(tr("Open As..."));
    openAsMenu->addAction(m_openFileAsText);
    openAsMenu->addAction(m_openFileAsJson);

    fileMenu->addSeparator();

    fileMenu->addAction(m_actSave);
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
    connect(m_actSave, &QAction::triggered, this, &MainWindow::saveConf);
    connect(m_tabViews, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
}

void MainWindow::loadConfig()
{
    const auto& confPaths = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    if (confPaths.isEmpty())
    {
        qCritical() << "Cannot locate the default configuration directory";
        return;
    }

    m_configDir.setPath(confPaths.first());
    if (!m_configDir.exists())
    {
        if (!m_configDir.mkpath(m_configDir.absolutePath()))
        {
            qCritical() << "Cannot create config directory" << m_configDir.absolutePath();
            return;
        }
    }

    m_templatesDir.setPath(m_configDir.absolutePath() + QDir::separator() + "templates");
    if (!m_templatesDir.exists())
    {
        if (!m_templatesDir.mkpath(m_templatesDir.absolutePath()))
        {
            qCritical() << "Cannot create config/templates directory" << m_templatesDir.absolutePath();
            return;
        }
    }

    const auto& templates = m_templatesDir.entryList(QDir::Files|QDir::Readable);

    auto fileMenu = menuBar()->addMenu(tr("&Templates"));
    for (const auto& templ : templates)
    {
        Conf *conf = new Conf(m_templatesDir.absoluteFilePath(templ).toStdString());
        QAction *actOpenTempl = new QAction(conf->getConfigName().c_str(), this);
        actOpenTempl->setProperty("conf", QVariant::fromValue(conf));
        connect(actOpenTempl, &QAction::triggered, this, &MainWindow::openTemplTriggered);
        fileMenu->addAction(actOpenTempl);
    }

    //auto fileMenu = menuBar()->addMenu(tr("&File"));

    // QFile confFile(configDir.absoluteFilePath("config.txt"));
    // if (!confFile.open(QIODevice::WriteOnly))
    // {
    //     qCritical() << "Cannot open config file" << confFile.fileName();
    //     return;
    // }
}

void MainWindow::openTemplTriggered()
{
    const auto senderAct(sender());
    if (senderAct == nullptr)
        return;
    
    auto conf = senderAct->property("conf").value<Conf*>();
    if (conf)
    {
        const auto fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
        if (fileName.isEmpty())
            return;
        auto newConf = new Conf(*conf);
        newConf->setFileName(fileName.toStdString());
        openFile(newConf);
    }
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
    openFile(conf);
}

void MainWindow::openFile(Conf* conf)
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
}

void MainWindow::closeTab(int index)
{
    LogTabWidget *tab = qobject_cast<LogTabWidget *>(m_tabViews->widget(index));
    if (!tab)
    {
        qCritical() << "No tab for index" << index;
        return;
    }

    m_tabViews->removeTab(index);
    delete tab;
    
}

void MainWindow::saveConf()
{
    LogTabWidget *tab = qobject_cast<LogTabWidget *>(m_tabViews->currentWidget());
    if (!tab)
    {
        qCritical() << "No current tab";
        return;
    }

    auto& conf = tab->getConf();
    if (conf.exists())
    {
        conf.saveConf();
    }
    else
    {
        bool ok;
        QString confName = QInputDialog::getText(this, tr("Save template"),
                                         tr("Template name:"), QLineEdit::Normal, QString(), &ok);
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
            while(m_templatesDir.exists(fileName + ext))
            {
                fileName.append(QString("_%1").arg(++cnt));
            }
            fileName.append(ext);
            
            fileName = m_templatesDir.absoluteFilePath(fileName);
            conf.saveConfAs(fileName.toStdString());
        }
    }
}

void MainWindow::testHeaderView()
{
    HeaderView *headerView = new HeaderView(this);
    setCentralWidget(headerView);
}

void MainWindow::testLogWidget()
{
    // TextLogModel
    // log.txt
    // biglog.txt
    // smalllog.txt

    // JsonLogModel
    // log.json
    // biglog.json
    // smalllog.json

    // echo '{"LogLevel":"TEST","DateTime":"28-12-2021 18:04:02.00205"}' >> /home/rafael/Dev/QLogViewer/log.json
    // echo '[T]: 28-12-2021 18:26:01.191 [test/cpp]: Test' >> /home/rafael/Dev/QLogViewer/log.txt

    // LogSearchWidget *lsw = new LogSearchWidget(this);
    // setCentralWidget(lsw);

    // BaseLogModel *model = new TextLogModel("/home/rafael/Dev/QLogViewer/log.txt", this);
    // LogViewWidget *mainLog = new LogViewWidget(this);
    // mainLog->setMinimumSize(400, 200);
    // mainLog->setLogModel(model);

    // createActions();
    // createMenus();
    // // createToolBars();
    // createConnections();

    // setCentralWidget(mainLog);

    // model->start();

    // connect(m_toggeFollowing, &QAction::toggled, model, &BaseLogModel::setFollowing);
    // connect(
    //     m_startSearch,
    //     &QAction::triggered,
    //     model,
    //     [model](bool)
    //     {
    //         SearchParamLst params;

    //         SearchParam param1;
    //         param1.isRegex = false;
    //         param1.wholeText = true;
    //         param1.exp = "555433422";
    //         // param1.column = 6;
    //         param1.matchCase = true;
    //         params.push_back(std::move(param1));

    //         // SearchParam param2;
    //         // param2.isRegex = false;    AbstractModel.cpp
    //         // param2.wholeText = false;
    //         // param2.exp = "TeS";
    //         // param2.column = 0;
    //         // param2.matchCase = false;
    //         // params.push_back(std::move(param2));

    //         model->startSearch(params);
    //     });

    // connect(m_stopSearch, &QAction::triggered, model, [model](bool) { model->stopSearch(); });
}

void MainWindow::testScrollBar()
{
    // LongScrollBar *lScrollBar = new LongScrollBar(Qt::Vertical, this);
    // lScrollBar->setMax(2147483647L*1000L);
    // //lScrollBar->setPos(2147483647/2);
    // setCentralWidget(lScrollBar);
}

void MainWindow::testFile()
{
    //    std::ofstream ofs("/home/rafael/Dev/QLogViewer/biglog.txt", std::ofstream::out | std::ofstream::app);
    //    ofs << "First Line" << std::endl;
    //    constexpr int64_t totalToWrite(2147483647L + 100L);
    //    constexpr int64_t notyfyPercentage(10);
    //    constexpr int64_t notifyEach(totalToWrite/notyfyPercentage);
    //    int64_t percentage(0);
    //    for (int64_t i = 0; i < totalToWrite; ++i)
    //    {
    //        if (((i+1) % notifyEach) == 0L)
    //        {
    //            percentage += notyfyPercentage;
    //            qDebug() << percentage << "%";
    //        }
    //        ofs << i << std::endl;
    //    }
    //    ofs << "Last Line" << std::endl;
    //    ofs.close();

    // std::string buffer;
    // buffer.resize(100);

    // std::ifstream s("/home/rafael/Dev/QLogViewer/log.txt");

    // size_t fileSize(0);
    // ssize_t pos(0);

    // s.seekg(0, std::ios::end);
    // fileSize = s.tellg();
    // s.seekg(0, std::ios::beg);

    // qDebug() << "fileSize" << fileSize;

    // pos = s.tellg();
    // qDebug() << pos;

    // while(!s.eof())
    // {
    //     s.read(buffer.data(), 100);
    //     //pos = s.gcount();
    //     pos = s.tellg();
    //     qDebug() << pos;
    // }
    // qDebug() << "gcount" << s.gcount();

    // // qDebug() << "good" << s.good();
    // // s.clear(std::ios::eofbit);
    // qDebug() << "good" << s.good();
    // s.clear(std::ios::eofbit);
    // s.seekg(0, std::ios::beg);
    // s.read(buffer.data(), 100);
    // qDebug() << "gcount" << s.gcount();
    // qDebug() << "good" << s.good();

    // pos = s.tellg();
    // qDebug() << pos;
}