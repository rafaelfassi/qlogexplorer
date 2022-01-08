#pragma once

#include <QMainWindow>

class QAction;

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

    void testLogWidget();
    void testScrollBar();
    void testFile();

private:
    QAction *m_toggeFollowing;
    QAction *m_startSearch;
    QAction *m_stopSearch;
};
