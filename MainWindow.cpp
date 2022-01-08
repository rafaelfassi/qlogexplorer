#include "MainWindow.h"
#include "LogViewWidget.h"
#include "TextLogModel.h"
#include "JsonLogModel.h"
#include "LongScrollBar.h"
#include "LogSearchWidget.h"
#include "LogTabWidget.h"
#include <QTableView>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QAction>
#include <QToolBar>
#include <QMenuBar>

#include <fstream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    LogTabWidget *logTabWidget = new LogTabWidget(this);
    setCentralWidget(logTabWidget);
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
    m_toggeFollowing = new QAction(tr("Follow..."), this);
    m_toggeFollowing->setCheckable(true);

    m_startSearch = new QAction(tr("Start Search"), this);
    m_stopSearch = new QAction(tr("Stop Search"), this);
    // addAction(m_toggeFollowing);
}

void MainWindow::createMenus()
{
    auto fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_toggeFollowing);
    fileMenu->addAction(m_startSearch);
    fileMenu->addAction(m_stopSearch);
}

void MainWindow::createToolBars()
{
    // auto fileToolBar = addToolBar(tr("&File"));
    // fileToolBar->addAction(m_toggeFollowing);
}

void MainWindow::createConnections()
{
    // connect(m_comboTestUnit, SIGNAL(currentIndexChanged(QString)), this, SLOT(selectTestUnit(QString)));
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