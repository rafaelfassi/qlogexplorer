// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QFormLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTabWidget;
class QToolButton;
class QVBoxLayout;
class QAction;
class QFrame;
class SearchParamControl;
class MainWindow;

class TemplatesConfigDlg : public QDialog
{
    Q_OBJECT

public:
    enum class OpenAction
    {
        None,
        AddFilter
    };
    TemplatesConfigDlg();
    static void setMainWindow(MainWindow *mainWindow);

    void setOpenActionAddFilter(const tp::FilterParam &param);
    void open() override;
    int exec() override;

private slots:
    // Main
    bool canSave() const;
    void save();
    bool canApply() const;
    void apply();
    void updateStatus();
    void updateCmbTemplates();
    void setCurrentTemplate(int index);
    void deleteTemplate();
    void createColumnsFromRegex();
    void updateTemplateMainInfo();
    // Columns
    void setCurrentColumn(int index);
    void setColumnType(int index);
    void updateTemplateColumns();
    void addColumn();
    void rmColumn();
    // Highlighters
    void setCurrentHighlighter(int index);
    void updateTemplateHighlighters();
    void addHighlighter();
    void rmHighlighter();
    void moveHighlighterUp();
    void moveHighlighterDown();
    void pickColor();
    // Filters
    void setCurrentFilter(int index);
    void updateTemplateFilter();
    void addFilter(tp::FilterParam newFlt = tp::FilterParam());
    void rmFilter();
    void moveFilterUp();
    void moveFilterDown();

private:
    void handleOpenActions();
    void updateTabs(bool currentOnly);
    FileConf::Ptr getCuttentTempl() const;
    void fillColumns(const FileConf::Ptr &conf, int selectRow = 0);
    void fillHighlighters(const FileConf::Ptr &conf, int selectRow = 0);
    void fillFilters(const FileConf::Ptr &conf, int selectRow = 0);
    void fixColumns(const FileConf::Ptr &conf);
    void createActions();
    void createConnections();
    void configureRegexMode(const FileConf::Ptr &conf);
    void buildLayout();

    // Data
    OpenAction m_openAction = OpenAction::None;
    tp::FilterParam m_newFilter;
    FileConf::Ptr m_conf;
    std::vector<FileConf::Ptr> m_templates;
    std::vector<FileConf::Ptr> m_deletedTemplates;

    // Actions ----------------------------------
    QAction *m_actDeleteTempl;
    QAction *m_actRunRegex;
    // Columns
    QAction *m_actAddColumn;
    QAction *m_actRmColumn;
    // Highlighters
    QAction *m_actAddHighlighter;
    QAction *m_actRmHighlighter;
    QAction *m_actMoveHighlighterUp;
    QAction *m_actMoveHighlighterDown;
    QAction *m_actHltForeColor;
    QAction *m_actHltBackColor;
    // Filters
    QAction *m_actAddFilter;
    QAction *m_actRmFilter;
    QAction *m_actMoveFilterUp;
    QAction *m_actMoveFilterDown;

    // Layout -----------------------------------
    QFormLayout *m_frmTemplMain;
    QFrame *m_frameTempl;
    QComboBox *m_cmbTemplates;
    QLabel *m_labFileType;
    QLineEdit *m_edtConfName;
    QHBoxLayout *m_hRegex;
    QLabel *m_labRegex;
    QLineEdit *m_edtRegex;
    QToolButton *m_btnRunRegex;
    QTabWidget *m_tabWidgets;
    // Columns tab
    QWidget *m_tabColumns;
    QListWidget *m_lstColumns;
    QFormLayout *m_frmColumn;
    QLineEdit *m_edtColName;
    QLabel *m_labColKey;
    QLineEdit *m_edtColKey;
    QComboBox *m_cmbColType;
    QLineEdit *m_edtColFormat;
    QCheckBox *m_chkNoMatchCol;
    // Highlighters tab
    QWidget *m_tabHighlighters;
    QListWidget *m_lstHighlighters;
    QComboBox *m_cmbHltColumn;
    QLineEdit *m_edtHltPattern;
    SearchParamControl *m_hltSearchCtrl;
    // Filters tab
    QWidget *m_tabFilters;
    QListWidget *m_lstFilters;
    QLineEdit *m_edtFltName;
    QComboBox *m_cmbFltColumn;
    QLineEdit *m_edtFltPattern;
    SearchParamControl *m_fltSearchCtrl;
    // Dialog buttons
    QPushButton *m_btnApply;
    QPushButton *m_btnSave;
    QPushButton *m_btnCancel;
};
