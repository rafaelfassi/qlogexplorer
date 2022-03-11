// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QWidget>

class QAction;
class QLineEdit;
class QComboBox;
class QCompleter;
class SearchParamModel;
class SearchParamProxyModel;

class SearchParamControl : public QWidget
{
    Q_OBJECT

public:
    SearchParamControl(QComboBox *cmbColumns, QLineEdit *edtPattern, QWidget *parent = nullptr);
    SearchParamControl(
        QComboBox *cmbColumns,
        QComboBox *cmbSearch,
        SearchParamModel *searchModel,
        QWidget *parent = nullptr);
    ~SearchParamControl() = default;
    void createActions();
    void createConnections();
    void setFileConf(FileConf::Ptr conf);
    tp::SearchParam getSearchParam() const;
    void setSearchParam(const tp::SearchParam &param, bool notifyChanged = false);
    static void fixParam(const FileConf::Ptr &conf, tp::SearchParam &param);
    void reconfigure();
    void retranslateUi();

signals:
    void paramChanged();
    void searchRequested();

public slots:
    void updateColumns(bool tryKeepSel);
    void apply();

private slots:
    void updateParam(bool notifyChanged = true);

private:
    void translateUi();

    tp::SearchParam m_param;
    QComboBox *m_cmbColumns;
    QLineEdit *m_edtPattern;
    QComboBox *m_cmbSearch;
    FileConf::Ptr m_conf;
    QAction *m_actRegex;
    QAction *m_actRange;
    QAction *m_actMatchCase;
    QAction *m_actNotOp;
    QCompleter *m_completer = nullptr;
    SearchParamProxyModel *m_proxyModel = nullptr;
};
