#pragma once

#include <QWidget>

class BaseLogModel;
class LogViewWidget;
class LogSearchWidget;

enum class FileType
{
    Text,
    Json
};

class LogTabWidget : public QWidget
{
    Q_OBJECT

public:
    LogTabWidget(const QString& fileName, FileType type, QWidget *parent = nullptr);
    ~LogTabWidget();
    void createActions();
    void createMenus();
    void createToolBars();
    void createConnections();

private:
    BaseLogModel *m_logModel;
    LogViewWidget *m_logViewWidget;
    LogSearchWidget *m_logSearchWidget;
};
