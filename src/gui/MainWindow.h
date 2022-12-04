// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QMainWindow>

class QMenu;
class QTimer;
class QAction;
class QTabWidget;
class LogTabWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateMenus();
    void retranslateUi();
    void reconfigure();

    int getTabsCount() const;
    int getCurrentTabIdx() const;
    LogTabWidget *getCurrentTab() const;
    LogTabWidget *getTab(int idx) const;

    int findOpenedFileTab(const FileConf::Ptr &conf);
    bool hasOpenedType(const FileConf::Ptr &conf);

    QStringList getFilesToOpen();
    void setFilesToOpen(const QStringList &files);
    void clearFilesToOpen();

public slots:
    void openFile(FileConf::Ptr conf, int idx = -1);
    void openFile(const QString &fileName, tp::FileType type);
    void openFiles(tp::FileType type);
    void openFiles(const QString &typeOrTemplateName);
    void reopenCurrentFile(FileConf::Ptr conf);
    void reopenCurrentFile(tp::FileType type);
    void closeTab(int index);
    void closeCurrentTab();
    void updateCurrentTab();
    void configAsCurrentTab(int index);
    void goToTab(int index);
    void saveConf();
    void saveConfAs();
    void openTemplatesConfig();
    void openSettings();
    void setRecentFile(const FileConf::Ptr &conf);
    void openWiki();
    void openAbout();
    void updateCurrentTabConf(FileConf::Ptr conf);
    void updateOpenedConf(FileConf::Ptr conf);
    void updateAllOpenedConfs();

private slots:
    void handleOpenFilesWithTemplate();
    void handleReopenWithTemplate();
    void handleOpenRecentFile();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createConnections();
    void translateUi();
    void updateTemplates();
    void updateRecentFiles();

private:
    QAction *m_actOpenFileAsText;
    QAction *m_actOpenFileAsJson;
    QAction *m_actReopenFileAsText;
    QAction *m_actReopenFileAsJson;
    QAction *m_actCloseFile;
    QAction *m_actQuit;
    QAction *m_actSaveConf;
    QAction *m_actSaveConfAs;
    QAction *m_actTemplatesConfig;
    QAction *m_actSettings;
    QAction *m_actOpenWiki;
    QAction *m_actAbout;
    QAction *m_actOpenAsSep;
    QMenu *m_fileMenu;
    QMenu *m_fileOpenAsMenu;
    QMenu *m_fileOpenRecent;
    QMenu *m_fileReopenAsMenu;
    QMenu *m_templatesMenu;
    QMenu *m_toolsMenu;
    QMenu *m_helpMenu;
    QTabWidget *m_tabViews;
    QStringList m_filesToOpen;
    QTimer *m_updateTimer;
    std::vector<std::pair<QAction *, FileConf::Ptr>> m_actRecentFiles;
    std::vector<std::pair<QAction *, FileConf::Ptr>> m_actOpenWithTemplates;
    std::vector<std::pair<QAction *, FileConf::Ptr>> m_actReopenWithTemplates;
};
