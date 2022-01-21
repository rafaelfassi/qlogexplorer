#pragma once

#include <QMainWindow>

class QAction;
class QTabWidget;
class LogTabWidget;

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
    void loadConfig();

public slots:
    void openFile(tp::FileType type);
    void openFile(const QString &fileName, tp::FileType type);
    void openFile(Conf* conf);
    void closeTab(int index);
    void saveConf();
    void openTemplTriggered();

private:
    void testHeaderView();
    void testLogWidget();
    void testScrollBar();
    void testFile();

private:
    QAction *m_openFile;
    QAction *m_openFileAsText;
    QAction *m_openFileAsJson;
    QAction *m_actSave;
    QTabWidget *m_tabViews;
    QDir m_configDir;
    QDir m_templatesDir;
    std::vector<Conf*> m_templates;
};
