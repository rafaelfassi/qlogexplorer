#include "MainWindow.h"
#include "LogView.h"
#include "TextLogModel.h"
#include "JsonLogModel.h"
#include <QTableView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    AbstractLogModel *model = new TextLogModel("/home/rafael/Dev/QLogViewer/biglog.txt", this);
    LogView *mainLog = new LogView(this);
    mainLog->setLogModel(model);
    setCentralWidget(mainLog);
    model->loadFile();
    model->startWatch();
}

MainWindow::~MainWindow()
{

}
