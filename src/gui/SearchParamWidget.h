// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QWidget>

class QAction;
class QLineEdit;
class QComboBox;
class SearchParamControl;

class SearchParamWidget : public QWidget
{
    Q_OBJECT

public:
    SearchParamWidget(FileConf::Ptr conf, QWidget *parent = nullptr);
    ~SearchParamWidget();
    void createActions();
    void createConnections();
    void updateColumns();
    bool getIsEnabled() const;
    tp::SearchParam getSearchParam() const;

signals:
    void searchRequested();
    void deleteRequested(QWidget *);

private:
    QAction *m_actDisableMe;
    QAction *m_actRemoveMe;
    QLineEdit *m_txtSearch;
    QComboBox *m_cmbColumns;
    SearchParamControl *m_control;
};
