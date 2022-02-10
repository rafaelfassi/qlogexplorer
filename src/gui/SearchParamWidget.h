// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

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
    tp::SearchType getSearchType();
    tp::SearchFlags getSearchFlags();
    bool getIsEnabled() const;
    std::string expression() const;
    std::optional<tp::Column> column() const;

private slots:
    void updateOptions();

signals:
    void searchRequested();
    void deleteRequested(QWidget *);

private:
    QAction *m_actMatchCase;
    QAction *m_actRegex;
    QAction *m_actRange;
    QAction *m_actNotOp;
    QAction *m_actDisableMe;
    QAction *m_actRemoveMe;
    QLineEdit *m_txtSearch;
    QComboBox *m_cmbColumns;
    tp::Columns m_columns;
    AbstractModel *m_model;
};
