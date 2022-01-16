#pragma once

#include <QWidget>

class QVBoxLayout;
class QAction;
class QComboBox;
class LogViewWidget;
class BaseLogModel;
class ProxyModel;
class SearchParamWidget;

class LogSearchWidget : public QWidget
{
    Q_OBJECT

public:
    LogSearchWidget(BaseLogModel* sourceModel, QWidget *parent = nullptr);
    ~LogSearchWidget();
    void createActions();
    void createMenus();
    void createToolBars();
    void createConnections();

signals:
    void rowSelected(ssize_t row);

public slots:
    void addSearchResult(std::shared_ptr<std::deque<ssize_t>> rowsPtr);

private slots:
    void addSearchParam();
    void startSearch();
    void clearResults();
    void deleteParamWidget(QWidget*);
    void sourceModelConfigured();

private:
    QAction *m_actAddSearchParam;
    QAction *m_actMergeResults;
    QAction *m_actOrOperator;
    QAction *m_actClear;
    QAction *m_actExec;
    QVBoxLayout *m_searchParamsLayout;
    LogViewWidget *m_searchResults;
    BaseLogModel *m_sourceModel;
    ProxyModel *m_proxyModel;
    QList<SearchParamWidget*> m_searchParamWidgets;
};
