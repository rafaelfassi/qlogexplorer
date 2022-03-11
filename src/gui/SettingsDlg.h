// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QDialogButtonBox;

class SettingsDlg : public QDialog
{
    Q_OBJECT

public:
    SettingsDlg(QWidget *parent = nullptr);

private slots:
    void save();
    void updateFontSizes(const QString &family);

private:
    void load();
    void createConnections();
    void buildLayout();

    QComboBox *m_cmbLang;
    QComboBox *m_cmbFontFamily;
    QComboBox *m_cmbFontSize;
    QComboBox *m_cmbTheme;
    QCheckBox *m_chkHideUniqueTab;
    QCheckBox *m_chkAllowMultiInst;
    QCheckBox *m_chkRegexAsDefault;
    QLineEdit *m_edtSettingsPath;
    QLineEdit *m_edtTemplatesPath;
    QDialogButtonBox *m_buttonBox;
};
