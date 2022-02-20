// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QMainWindow>

class QMenu;
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
    void openFile(FileConf::Ptr conf);
    void openFile(const QString &fileName, tp::FileType type);
    void openFiles(tp::FileType type);
    void openFiles(const QString &typeOrTemplateName);
    void closeTab(int index);
    void confCurrentTab(int index);
    void goToTab(int index);
    void saveConf();
    void saveConfAs();
    void openTemplatesConfig();
    void setRecentFile(const FileConf::Ptr &conf);

private slots:
    void handleOpenWithTemplate();
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
    void updateTemplates();
    void updateRecentFiles();

private:
    QAction *m_openFileAsText;
    QAction *m_openFileAsJson;
    QAction *m_actSaveConf;
    QAction *m_actSaveConfAs;
    QAction *m_actTemplatesConfig;
    QAction *m_actRecentFilesSep;
    QAction *m_actOpenAsSep;
    QMenu *m_fileMenu;
    QMenu *m_fileOpenAsMenu;
    QMenu *m_templatesMenu;
    QTabWidget *m_tabViews;
    QStringList m_filesToOpen;
    std::vector<std::pair<QAction *, FileConf::Ptr>> m_actRecentFiles;
    std::vector<std::pair<QAction *, FileConf::Ptr>> m_actTemplates;
};
