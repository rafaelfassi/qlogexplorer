#pragma once

#include <QMainWindow>
#include "LogTabWidget.h"

class QAction;
class QTabWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void createActions();
    void createMenus();
    void createToolBars();
    void createConnections();

public slots:
    void openFile(FileType type);
    void openFile(const QString &fileName, FileType type);
    void closeTab(int index);

private:
    void testHeaderView();
    void testLogWidget();
    void testScrollBar();
    void testFile();

private:
    QAction *m_openFile;
    QAction *m_openFileAsText;
    QAction *m_openFileAsJson;
    QTabWidget *m_tabViews;
};
