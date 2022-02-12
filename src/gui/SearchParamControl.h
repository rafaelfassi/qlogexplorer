// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QWidget>

class QAction;
class QLineEdit;
class QComboBox;

class SearchParamControl : public QWidget
{
    Q_OBJECT

public:
    SearchParamControl(QComboBox *cmbColumns, QLineEdit *edtPattern, QWidget *parent = nullptr);
    ~SearchParamControl();
    void createActions();
    void createConnections();
    void setFileConf(FileConf::Ptr conf);
    tp::SearchParam getSearchParam() const;
    void setSearchParam(const tp::SearchParam &param, bool notifyChanged = false);

signals:
    void paramChanged();
    void searchRequested();

public slots:
    void updateColumns(bool tryKeepSel);

private slots:
    void updateParam(bool notifyChanged = true);

private:
    tp::SearchParam m_param;
    QComboBox *m_cmbColumns;
    QLineEdit *m_edtPattern;
    FileConf::Ptr m_conf;
    QAction *m_actRegex;
    QAction *m_actRange;
    QAction *m_actMatchCase;
    QAction *m_actNotOp;
};
