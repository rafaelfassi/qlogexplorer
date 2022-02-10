// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QMainWindow>
#include "Settings.h"

class QMenu;
class QAction;
class QTabWidget;
class QSettings;
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
    void updateTemplates();

public slots:
    void openFile(tp::FileType type);
    void openFile(const QString &fileName, tp::FileType type);
    void openFile(Conf *conf);
    void closeTab(int index);
    void confCurrentTab(int index);
    void goToTab(int index);
    void saveConf();
    void saveConfAs();
    void editRegex();
    void setRecentFile(Conf *conf);

private slots:
    void handleOpenWithTemplate();
    void handleOpenRecentFile();

private:
    void updateRecentFiles();
    int findOpenedFileTab(const Conf &conf);

private:
    QAction *m_openFile;
    QAction *m_openFileAsText;
    QAction *m_openFileAsJson;
    QAction *m_actSaveConf;
    QAction *m_actSaveConfAs;
    QAction *m_actEdtRegex;
    QAction *m_actRecentFilesSep;
    QAction *m_actOpenAsSep;
    QMenu *m_fileMenu;
    QMenu *m_fileOpenAsMenu;
    std::vector<std::pair<QAction *, Conf>> m_actRecentFiles;
    std::vector<std::pair<QAction *, Conf *>> m_actTemplates;
    QTabWidget *m_tabViews;
};
