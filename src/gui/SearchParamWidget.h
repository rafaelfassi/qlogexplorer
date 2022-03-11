// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QWidget>

class QAction;
class QLineEdit;
class QComboBox;
class SearchParamModel;
class SearchParamControl;

class SearchParamWidget : public QWidget
{
    Q_OBJECT

public:
    SearchParamWidget(FileConf::Ptr conf, SearchParamModel *searchModel, QWidget *parent = nullptr);
    ~SearchParamWidget();
    void apply();
    void reconfigure();
    void retranslateUi();
    bool getIsEnabled() const;
    tp::SearchParam getSearchParam() const;

signals:
    void searchRequested();
    void deleteRequested(QWidget *);

private:
    void createActions();
    void createConnections();
    void translateUi();

    QAction *m_actDisableMe;
    QAction *m_actRemoveMe;
    QComboBox *m_cmbSearch;
    QComboBox *m_cmbColumns;
    SearchParamControl *m_control;
};
