#pragma once

#include <QWidget>

class QAction;
class QLineEdit;
class QComboBox;
class AbstractModel;

class SearchParamWidget : public QWidget
{
    Q_OBJECT

public:
    SearchParamWidget(AbstractModel *sourceModel, QWidget *parent = nullptr);
    ~SearchParamWidget();
    void createActions();
    void createConnections();
    void setColumns(const tp::Columns &columns);
    bool matchCase() const;
    bool isRegex() const;
    bool notOp() const;
    bool getIsEnabled() const;
    std::string expression() const;
    std::optional<tp::UInt> column() const;

signals:
    void searchRequested();
    void deleteRequested(QWidget *);

private:
    QAction *m_actMatchCase;
    QAction *m_actRegex;
    QAction *m_actNotOp;
    QAction *m_actDisableMe;
    QAction *m_actRemoveMe;
    QLineEdit *m_txtSearch;
    QComboBox *m_cmbColumns;
    AbstractModel *m_model;
};
