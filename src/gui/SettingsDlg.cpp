// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "SettingsDlg.h"
#include "Settings.h"
#include "Style.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QFontDatabase>

SettingsDlg::SettingsDlg(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    buildLayout();
    load();
    createConnections();
}

void SettingsDlg::load()
{
    m_cmbLang->addItems(Settings::availableLangs());
    if (const auto idx = m_cmbLang->findText(Settings::getLanguage()); idx != -1)
    {
        m_cmbLang->setCurrentIndex(idx);
    }

    QFontDatabase database;
    for (const auto &family : database.families())
    {
        if (database.isFixedPitch(family))
            m_cmbFontFamily->addItem(family);
    }

    const auto &fontFamily = Settings::getFont().family();
    if (const auto idx = m_cmbFontFamily->findText(fontFamily); idx != -1)
    {
        m_cmbFontFamily->setCurrentIndex(idx);
        updateFontSizes(fontFamily);
    }

    m_cmbTheme->addItems(Style::availableStyles());
    if (const auto idx = m_cmbTheme->findText(Settings::getStyle()); idx != -1)
    {
        m_cmbTheme->setCurrentIndex(idx);
    }

    m_chkHideUniqueTab->setChecked(Settings::getHideUniqueTab());

    m_chkAllowMultiInst->setChecked(!Settings::getSingleInstance());

    m_chkRegexAsDefault->setChecked(Settings::getDefaultSearchType() == tp::SearchType::Regex);

    m_edtSettingsPath->setText(Settings::getSettingsDir().absolutePath());
    m_edtTemplatesPath->setText(Settings::gettemplatesDir().absolutePath());
}

void SettingsDlg::updateFontSizes(const QString &family)
{
    QString oriSize = m_cmbFontSize->currentText();
    if (oriSize.isEmpty())
    {
        oriSize = QString::number(Settings::getFont().pointSize());
    }

    QFontDatabase database;
    QList<int> sizes = database.pointSizes(family);
    if (sizes.empty())
    {
        sizes = QFontDatabase::standardSizes();
    }

    m_cmbFontSize->clear();
    for (auto size : sizes)
    {
        m_cmbFontSize->addItem(QString::number(size));
    }

    if (const auto idx = m_cmbFontSize->findText(oriSize); idx != -1)
    {
        m_cmbFontSize->setCurrentIndex(idx);
    }
}

void SettingsDlg::save()
{
    const auto lang = m_cmbLang->currentText();
    if (!lang.isEmpty())
    {
        Settings::setLanguage(lang);
    }

    const auto fontFamily = m_cmbFontFamily->currentText();
    const auto fontSizeStr = m_cmbFontSize->currentText();
    if (!fontFamily.isEmpty() && !fontSizeStr.isEmpty())
    {
        bool ok;
        const auto fontSize = fontSizeStr.toInt(&ok);
        const auto &font = Settings::getFont();
        if (ok && ((font.family() != fontFamily) || (font.pointSize() != fontSize)))
        {
            Settings::setFont(fontFamily, fontSize);
        }
    }

    const auto theme = m_cmbTheme->currentText();
    if (!theme.isEmpty() && theme != Settings::getStyle())
    {
        Settings::setStyle(theme);
        Style::loadStyle(theme);
    }

    if (Settings::getHideUniqueTab() != m_chkHideUniqueTab->isChecked())
    {
        Settings::setHideUniqueTab(m_chkHideUniqueTab->isChecked());
    }

    const bool singleInst = !m_chkAllowMultiInst->isChecked();
    if (Settings::getSingleInstance() != singleInst)
    {
        Settings::setSingleInstance(singleInst);
    }

    const auto searchType = m_chkRegexAsDefault->isChecked() ? tp::SearchType::Regex : tp::SearchType::SubString;
    if (Settings::getDefaultSearchType() != searchType)
    {
        Settings::setDefaultSearchType(searchType);
    }

    QDialog::accept();
}

void SettingsDlg::createConnections()
{
    connect(m_cmbFontFamily, &QComboBox::currentTextChanged, this, &SettingsDlg::updateFontSizes);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SettingsDlg::save);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void SettingsDlg::buildLayout()
{
    if (auto parWidget = qobject_cast<QWidget *>(parent()); parWidget != nullptr)
    {
        resize(parWidget->width() - 60, parWidget->height() - 60);
        move(parWidget->pos().x() + 30, parWidget->pos().y() + 30);
    }
    else
    {
        resize(600, 500);
    }

    // Root layout -------------------------------------------------------------------------------- (Start)
    auto vLayoutMain = new QVBoxLayout(this);

    // Locale --------------------------------------------------------------------------- (Start)
    auto grLocale = new QGroupBox(tr("Locale"), this);
    auto frmLocale = new QFormLayout(grLocale);
    m_cmbLang = new QComboBox(grLocale);
    frmLocale->addRow(tr("Language"), m_cmbLang);
    vLayoutMain->addWidget(grLocale);
    // Locale --------------------------------------------------------------------------- (End)

    // Font ----------------------------------------------------------------------------- (Start)
    auto grFont = new QGroupBox(tr("Font"), this);
    auto hFont = new QHBoxLayout(grFont);

    // Font Family
    auto frmFontFamily = new QFormLayout();
    m_cmbFontFamily = new QComboBox(grFont);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_cmbFontFamily->sizePolicy().hasHeightForWidth());
    m_cmbFontFamily->setSizePolicy(sizePolicy);
    frmFontFamily->addRow(tr("Family"), m_cmbFontFamily);
    hFont->addLayout(frmFontFamily);

    // Font Size
    auto frmFontSize = new QFormLayout();
    m_cmbFontSize = new QComboBox(grFont);
    frmFontSize->addRow(tr("Size"), m_cmbFontSize);
    hFont->addLayout(frmFontSize);

    vLayoutMain->addWidget(grFont);
    // Font ----------------------------------------------------------------------------- (End)

    // Appearance ----------------------------------------------------------------------- (Start)
    auto grAppearance = new QGroupBox(tr("Appearance"), this);
    auto frmAppearance = new QFormLayout(grAppearance);
    m_cmbTheme = new QComboBox(grAppearance);
    frmAppearance->addRow(tr("Theme"), m_cmbTheme);
    vLayoutMain->addWidget(grAppearance);
    // Appearance ----------------------------------------------------------------------- (End)

    // Behavior ------------------------------------------------------------------------- (Start)
    auto grBehavior = new QGroupBox(tr("Behavior"), this);
    auto vBehavior = new QVBoxLayout(grBehavior);

    m_chkHideUniqueTab = new QCheckBox(tr("Hide tabs when only one file"), grBehavior);
    vBehavior->addWidget(m_chkHideUniqueTab);

    m_chkAllowMultiInst = new QCheckBox(tr("Allow more than on instance"), grBehavior);
    vBehavior->addWidget(m_chkAllowMultiInst);

    m_chkRegexAsDefault = new QCheckBox(tr("Use regex as default search"), grBehavior);
    vBehavior->addWidget(m_chkRegexAsDefault);

    vLayoutMain->addWidget(grBehavior);
    // Behavior ------------------------------------------------------------------------- (End)

    // Paths ---------------------------------------------------------------------------- (Start)
    auto grPaths = new QGroupBox(tr("Paths"), this);
    auto frmPaths = new QFormLayout(grPaths);

    m_edtSettingsPath = new QLineEdit(grPaths);
    m_edtSettingsPath->setReadOnly(true);
    frmPaths->addRow(tr("Settings"), m_edtSettingsPath);

    m_edtTemplatesPath = new QLineEdit(grPaths);
    m_edtTemplatesPath->setReadOnly(true);
    frmPaths->addRow(tr("Templates"), m_edtTemplatesPath);

    vLayoutMain->addWidget(grPaths);
    // Paths ---------------------------------------------------------------------------- (End)

    auto vSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vLayoutMain->addItem(vSpacer);
    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->setOrientation(Qt::Horizontal);
    m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    vLayoutMain->addWidget(m_buttonBox);
    // Root layout -------------------------------------------------------------------------------- (End)
}
