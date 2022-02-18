// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "TemplatesConfigDlg.h"
#include "SearchParamControl.h"
#include "Settings.h"
#include "Style.h"
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QIntValidator>
#include <QColorDialog>

TemplatesConfigDlg::TemplatesConfigDlg(FileConf::Ptr conf, QWidget *parent) : QDialog(parent), m_conf(conf)
{
    setWindowTitle(tr("Templates Configuration"));
    createActions();
    buildLayout();

    if (!m_conf->isNull())
    {
        m_templates.push_back(FileConf::clone(m_conf));
    }

    auto templs = Settings::getTemplates();
    for (auto &templ : templs)
    {
        if (!templ->isSameType(m_conf))
        {
            m_templates.push_back(FileConf::clone(templ));
        }
    }

    m_cmbColType->addItem("String", tp::toInt(tp::ColumnType::Str));
    m_cmbColType->addItem("Signed Integer", tp::toInt(tp::ColumnType::Int));
    m_cmbColType->addItem("Unsigned Integer", tp::toInt(tp::ColumnType::UInt));
    m_cmbColType->addItem("Floating Point", tp::toInt(tp::ColumnType::Float));
    m_cmbColType->addItem("Date Time", tp::toInt(tp::ColumnType::Time));

    updateCmbTemplates();

    if (!m_templates.empty())
    {
        setCurrentTemplate(0);
    }

    createConnections();
}

FileConf::Ptr TemplatesConfigDlg::getCuttentTempl() const
{
    const auto idx = m_cmbTemplates->currentIndex();
    if (idx >= 0 && idx < m_templates.size())
    {
        return m_templates.at(idx);
    }
    return FileConf::make();
}

bool TemplatesConfigDlg::canSave() const
{
    bool canSave(!m_deletedTemplates.empty());

    for (auto &templ : m_templates)
    {
        if (canSave)
            break;

        if (!m_conf->isNull() && templ->isSameType(m_conf))
        {
            if (templ->exists())
            {
                canSave = !templ->isEqual(m_conf);
            }
            else
            {
                canSave = !templ->getConfigName().empty();
            }
        }
        else
        {
            auto conf = Settings::findConfByTemplateFileName(templ->getConfFileName());
            if (!conf->isNull())
            {
                canSave = !conf->isEqual(templ);
            }
        }
    }

    return canSave;
}

void TemplatesConfigDlg::save()
{
    for (auto &templ : m_templates)
    {
        if (!m_conf->isNull() && templ->isSameType(m_conf))
        {
            m_conf->copyFrom(templ);
            fixColumns(m_conf);
            if (!m_conf->exists())
            {
                if (!m_conf->getConfigName().empty())
                {
                    Settings::saveTemplateAs(m_conf, m_conf->getConfigName().c_str());
                    Settings::setRecentFile(m_conf);
                }
            }
            else
            {
                Settings::saveTemplate(m_conf);
            }
            continue;
        }

        auto conf = Settings::findConfByTemplateFileName(templ->getConfFileName());
        if (!conf->isNull())
        {
            // Has changed?
            if (!conf->isEqual(templ))
            {
                conf->copyFrom(templ);
                fixColumns(conf);
                Settings::saveTemplate(conf);
            }
        }
    }

    for (auto delTempl : m_deletedTemplates)
    {
        Settings::deleteTemplate(delTempl);
    }

    accept();
}

void TemplatesConfigDlg::apply()
{
    auto conf = getCuttentTempl();
    if (!m_conf->isNull() && conf->isSameType(m_conf))
    {
        m_conf->copyFrom(conf);
        fixColumns(m_conf);
        accept();
    }
}

void TemplatesConfigDlg::updateStatus()
{
    const auto conf = getCuttentTempl();
    const bool isCurrentFile(!m_conf->isNull() && conf->isSameType(m_conf));
    m_actDeleteTempl->setEnabled(!isCurrentFile && conf->exists());
    m_btnApply->setEnabled(isCurrentFile && !conf->isEqual(m_conf));
    m_btnSave->setEnabled(canSave());
}

void TemplatesConfigDlg::updateCmbTemplates()
{
    m_cmbTemplates->clear();

    if (!m_templates.empty())
    {
        for (auto &templ : m_templates)
        {
            QString name;
            if (!m_conf->isNull() && templ->isSameType(m_conf))
                name = "* ";
            if (templ->exists())
                name.append(templ->getConfigName().c_str());
            else
                name.append("<new>");
            m_cmbTemplates->addItem(name);
        }
        m_cmbTemplates->setCurrentIndex(0);
    }
}

void TemplatesConfigDlg::setCurrentTemplate(int index)
{
    if (index < 0)
    {
        m_frameTempl->setVisible(false);
        updateStatus();
        return;
    }
    m_frameTempl->setVisible(true);

    const auto &conf = m_templates.at(index);
    m_edtFileType->setText(tp::toStr(conf->getFileType()).c_str());
    m_edtConfName->setText(conf->getConfigName().c_str());
    m_hltSearchCtrl->setFileConf(conf);
    m_fltSearchCtrl->setFileConf(conf);
    configureRegexMode(conf);
    fillColumns(conf);
    fillHighlighters(conf);
    fillFilters(conf);
}

void TemplatesConfigDlg::deleteTemplate()
{
    const auto idx = m_cmbTemplates->currentIndex();
    if (idx >= 0 && idx < m_templates.size())
    {
        m_deletedTemplates.push_back(m_templates.at(idx));
        m_templates.erase(m_templates.begin() + idx);
        updateCmbTemplates();
        if (!m_templates.empty())
            setCurrentTemplate(0);
    }
}

void TemplatesConfigDlg::createColumnsFromRegex()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    if (conf->hasDefinedColumns())
    {
        QString fixHltMsg;
        if (conf->hasHighlighterParams())
        {
            fixHltMsg = tr("You may need to fix highlighters that are refereicing the existent columns.\n");
        }
        if (QMessageBox::warning(
                this,
                tr("Warning"),
                tr("All columns will be deleted and recreated.\n%1Do you want to continue?").arg(fixHltMsg),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No) != QMessageBox::Yes)
            return;
    }

    QRegularExpression rx(m_edtRegex->text());
    if (!rx.isValid())
    {
        QMessageBox::critical(this, "Error", tr("The regular expression is not valid.\n%1").arg(rx.errorString()));
        return;
    }

    conf->clearColumns();

    const auto groupsCount = rx.captureCount();
    const auto &namedGroups = rx.namedCaptureGroups();
    for (auto g = 1; g <= groupsCount; ++g)
    {
        tp::Column cl(g - 1);
        cl.key = std::to_string(g);

        if (g < namedGroups.size())
        {
            cl.name = namedGroups.at(g).toStdString();
        }

        if (cl.name.empty())
        {
            cl.name = cl.key;
        }

        conf->addColumn(std::move(cl));
    }

    if (conf->getColumns().empty())
        conf->addColumn(tp::Column(0));

    conf->setNoMatchColumn(0);

    fillColumns(conf);
}

void TemplatesConfigDlg::updateTemplateMainInfo()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    conf->setConfigName(utl::toStr(m_edtConfName->text()));
    conf->setRegexPattern(utl::toStr(m_edtRegex->text()));
    updateStatus();
}

void TemplatesConfigDlg::fillColumns(const FileConf::Ptr &conf, int selectRow)
{
    m_lstColumns->clear();
    if (conf->hasDefinedColumns())
    {
        for (const auto &col : conf->getColumns())
        {
            m_lstColumns->addItem(col.name.c_str());
        }
        if (selectRow >= m_lstColumns->count())
            selectRow = 0;

        setCurrentColumn(selectRow);
        m_lstColumns->setCurrentRow(selectRow);
    }
    else
    {
        setCurrentColumn(-1);
        m_lstColumns->setCurrentRow(-1);
    }

    m_hltSearchCtrl->updateColumns(true);
    m_fltSearchCtrl->updateColumns(true);
}

void TemplatesConfigDlg::setCurrentColumn(int index)
{
    auto enableColumnFormFunc = [this](bool enable)
    {
        m_edtColName->setEnabled(enable);
        m_edtColKey->setEnabled(enable);
        m_cmbColType->setEnabled(enable);
        m_chkNoMatchCol->setEnabled(enable);

        if (!enable)
        {
            m_edtColName->clear();
            m_edtColKey->clear();
            m_cmbColType->setCurrentIndex(0);
            m_edtColFormat->clear();
            m_edtColFormat->setEnabled(false);
            m_chkNoMatchCol->setChecked(false);
        }
    };

    const auto conf = getCuttentTempl();
    if (conf->isNull())
    {
        enableColumnFormFunc(false);
        return;
    }

    if (conf->hasDefinedColumn(index))
    {
        const auto &col = conf->getColumns().at(index);
        m_edtColName->setText(col.name.c_str());
        m_edtColKey->setText(col.key.c_str());
        const auto cmbColTypeIdx = m_cmbColType->findData(tp::toInt(col.type));
        if (cmbColTypeIdx != -1)
        {
            m_cmbColType->setCurrentIndex(cmbColTypeIdx);
        }
        m_edtColFormat->setText(col.format.c_str());
        m_chkNoMatchCol->setChecked(index == conf->getNoMatchColumn());
        enableColumnFormFunc(true);
        setColumnType(m_cmbColType->currentIndex());
    }
    else
    {
        enableColumnFormFunc(false);
    }

    updateStatus();
}

void TemplatesConfigDlg::setColumnType(int index)
{
    if (index != -1)
    {
        const auto colType = tp::fromInt<tp::ColumnType>(m_cmbColType->itemData(index).toInt());
        m_edtColFormat->setEnabled(colType == tp::ColumnType::Time);
        return;
    }
    m_edtColFormat->setEnabled(false);
}

void TemplatesConfigDlg::updateTemplateColumns()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    const int colIdx = m_lstColumns->currentRow();
    if (conf->hasDefinedColumn(colIdx))
    {
        auto &col = conf->getColumns().at(colIdx);
        col.name = utl::toStr(m_edtColName->text());
        col.key = utl::toStr(m_edtColKey->text());
        col.type = tp::fromInt<tp::ColumnType>(m_cmbColType->currentData().toInt());
        col.format = utl::toStr(m_edtColFormat->text());
        if (m_chkNoMatchCol->isChecked())
        {
            conf->setNoMatchColumn(colIdx);
        }

        auto item = m_lstColumns->item(colIdx);
        if (item != nullptr)
        {
            item->setText(m_edtColName->text());
        }
    }

    m_hltSearchCtrl->updateColumns(true);
    m_fltSearchCtrl->updateColumns(true);
    updateStatus();
}

void TemplatesConfigDlg::addColumn()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    if (!conf->hasDefinedColumns())
        conf->clearColumns();

    tp::Column newCol(conf->getColumns().size());
    const auto newColPos(newCol.idx + 1);

    newCol.name = utl::toStr(tr("New Column %1").arg(newColPos));

    if (conf->getFileType() == tp::FileType::Text)
        newCol.key = std::to_string(newColPos);
    else
        newCol.key = fmt::format("key_{}", newColPos);

    conf->addColumn(std::move(newCol));
    fillColumns(conf, conf->getColumns().size() - 1);
}

void TemplatesConfigDlg::rmColumn()
{
    auto conf = getCuttentTempl();
    if (conf->isNull() || !conf->hasDefinedColumns())
        return;

    const int colIdx = m_lstColumns->currentRow();
    if (conf->hasDefinedColumn(colIdx))
    {
        auto &columns(conf->getColumns());
        columns.erase(columns.begin() + colIdx);
        fillColumns(conf, colIdx < columns.size() ? colIdx : colIdx - 1);
    }
}

void TemplatesConfigDlg::fillHighlighters(const FileConf::Ptr &conf, int selectRow)
{
    m_lstHighlighters->clear();
    if (!conf->getHighlighterParams().empty())
    {
        for (const auto &hlt : conf->getHighlighterParams())
        {
            m_lstHighlighters->addItem(hlt.searchParam.pattern.c_str());
            auto item = m_lstHighlighters->item(m_lstHighlighters->count() - 1);
            item->setForeground(hlt.color.fg);
            item->setBackground(hlt.color.bg);
        }
        setCurrentHighlighter(selectRow);
        m_lstHighlighters->setCurrentRow(selectRow);
    }
    else
    {
        setCurrentHighlighter(-1);
        m_lstHighlighters->setCurrentRow(-1);
    }
}

void TemplatesConfigDlg::fixColumns(const FileConf::Ptr &conf)
{
    for (auto &hlt : conf->getHighlighterParams())
    {
        SearchParamControl::fixParam(conf, hlt.searchParam);
    }

    for (auto &flt : conf->getFilterParams())
    {
        SearchParamControl::fixParam(conf, flt.searchParam);
    }
}

void TemplatesConfigDlg::setCurrentHighlighter(int index)
{
    auto enableHltFormFunc = [this](bool enable)
    {
        m_cmbHltColumn->setEnabled(enable);
        m_edtHltPattern->setEnabled(enable);
        m_hltSearchCtrl->setEnabled(enable);
        m_actHltForeColor->setEnabled(enable);
        m_actHltBackColor->setEnabled(enable);

        if (!enable)
        {
            const auto fgColor(palette().color(QPalette::Text));
            const auto bgColor(palette().color(QPalette::Base));
            m_actHltForeColor->setIcon(Style::makeIcon(fgColor));
            m_actHltForeColor->setData(fgColor);
            m_actHltBackColor->setIcon(Style::makeIcon(bgColor));
            m_actHltBackColor->setData(bgColor);
            m_hltSearchCtrl->setSearchParam(tp::SearchParam());
        }
    };

    const auto conf = getCuttentTempl();
    if (conf->isNull())
    {
        enableHltFormFunc(false);
        return;
    }

    if (conf->hasHighlighterParam(index))
    {
        const auto &hit = conf->getHighlighterParams().at(index);
        m_actHltForeColor->setIcon(Style::makeIcon(hit.color.fg));
        m_actHltForeColor->setData(hit.color.fg);
        m_actHltBackColor->setIcon(Style::makeIcon(hit.color.bg));
        m_actHltBackColor->setData(hit.color.bg);
        m_hltSearchCtrl->setSearchParam(hit.searchParam);
        enableHltFormFunc(true);
    }
    else
    {
        enableHltFormFunc(false);
    }

    updateStatus();
}

void TemplatesConfigDlg::updateTemplateHighlighters()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    const int hltIdx = m_lstHighlighters->currentRow();
    if (conf->hasHighlighterParam(hltIdx))
    {
        auto &hit = conf->getHighlighterParams().at(hltIdx);
        auto colorFg = m_actHltForeColor->data().value<QColor>();
        auto colorBg = m_actHltBackColor->data().value<QColor>();
        hit.searchParam = m_hltSearchCtrl->getSearchParam();
        hit.color.fg = m_actHltForeColor->data().value<QColor>();
        hit.color.bg = m_actHltBackColor->data().value<QColor>();

        auto item = m_lstHighlighters->item(hltIdx);
        if (item != nullptr)
        {
            item->setText(hit.searchParam.pattern.c_str());
            item->setForeground(hit.color.fg);
            item->setBackground(hit.color.bg);
        }
    }
    updateStatus();
}

void TemplatesConfigDlg::addHighlighter()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    tp::HighlighterParam newHlt;
    const auto newHltPos(conf->getHighlighterParams().size() + 1);
    newHlt.searchParam.pattern = utl::toStr(tr("New Highlighter %1").arg(newHltPos));
    newHlt.color.fg = palette().color(QPalette::Text);
    newHlt.color.bg = palette().color(QPalette::Base);
    conf->addHighlighterParam(std::move(newHlt));

    fillHighlighters(conf, conf->getHighlighterParams().size() - 1);
}

void TemplatesConfigDlg::rmHighlighter()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    const int hltIdx = m_lstHighlighters->currentRow();
    if (conf->hasHighlighterParam(hltIdx))
    {
        auto &highlighters(conf->getHighlighterParams());
        highlighters.erase(highlighters.begin() + hltIdx);
        fillHighlighters(conf, hltIdx < highlighters.size() ? hltIdx : hltIdx - 1);
    }
}

void TemplatesConfigDlg::moveHighlighterUp()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    const int hltIdx = m_lstHighlighters->currentRow();
    if (conf->hasHighlighterParam(hltIdx) && (hltIdx > 0))
    {
        auto &highlighters(conf->getHighlighterParams());
        std::swap(highlighters[hltIdx], highlighters[hltIdx - 1]);
        fillHighlighters(conf, hltIdx - 1);
    }
}

void TemplatesConfigDlg::moveHighlighterDown()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    const int hltIdx = m_lstHighlighters->currentRow();
    auto &highlighters(conf->getHighlighterParams());
    if (conf->hasHighlighterParam(hltIdx) && (hltIdx < highlighters.size() - 1))
    {
        std::swap(highlighters[hltIdx], highlighters[hltIdx + 1]);
        fillHighlighters(conf, hltIdx + 1);
    }
}

void TemplatesConfigDlg::pickColor()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action == nullptr)
        return;

    auto color = QColorDialog::getColor(action->data().value<QColor>(), this);
    if (color.isValid())
    {
        action->setIcon(Style::makeIcon(color));
        action->setData(color);
        updateTemplateHighlighters();
    }
}

void TemplatesConfigDlg::fillFilters(const FileConf::Ptr &conf, int selectRow)
{
    m_lstFilters->clear();
    if (conf->hasFilterParams())
    {
        for (const auto &flt : conf->getFilterParams())
        {
            m_lstFilters->addItem(flt.name.c_str());
        }
        setCurrentFilter(selectRow);
        m_lstFilters->setCurrentRow(selectRow);
    }
    else
    {
        setCurrentFilter(-1);
        m_lstFilters->setCurrentRow(-1);
    }
}

void TemplatesConfigDlg::setCurrentFilter(int index)
{
    auto enableFltFormFunc = [this](bool enable)
    {
        m_edtFltName->setEnabled(enable);
        m_cmbFltColumn->setEnabled(enable);
        m_edtFltPattern->setEnabled(enable);
        m_fltSearchCtrl->setEnabled(enable);

        if (!enable)
        {
            m_edtFltName->clear();
            m_fltSearchCtrl->setSearchParam(tp::SearchParam());
        }
    };

    const auto conf = getCuttentTempl();
    if (conf->isNull())
    {
        enableFltFormFunc(false);
        return;
    }

    if (conf->hasFilterParam(index))
    {
        const auto &flt = conf->getFilterParams().at(index);
        m_edtFltName->setText(flt.name.c_str());
        m_fltSearchCtrl->setSearchParam(flt.searchParam);
        enableFltFormFunc(true);
    }
    else
    {
        enableFltFormFunc(false);
    }

    updateStatus();
}

void TemplatesConfigDlg::updateTemplateFilter()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    const int fltIdx = m_lstFilters->currentRow();
    if (conf->hasFilterParam(fltIdx))
    {
        auto &flt = conf->getFilterParams().at(fltIdx);
        flt.name = utl::toStr(m_edtFltName->text());
        flt.searchParam = m_fltSearchCtrl->getSearchParam();

        auto item = m_lstFilters->item(fltIdx);
        if (item != nullptr)
        {
            item->setText(flt.name.c_str());
        }
    }
    updateStatus();
}

void TemplatesConfigDlg::addFilter()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    tp::FilterParam newFlt;
    const auto newFltPos(conf->getFilterParams().size() + 1);
    newFlt.name = utl::toStr(tr("New Filter %1").arg(newFltPos));
    newFlt.searchParam.pattern = utl::toStr(tr("Pattern Filter %1").arg(newFltPos));
    conf->addFilterParam(std::move(newFlt));

    fillFilters(conf, conf->getFilterParams().size() - 1);
}

void TemplatesConfigDlg::rmFilter()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    const int fltIdx = m_lstFilters->currentRow();
    if (conf->hasFilterParam(fltIdx))
    {
        auto &filters(conf->getFilterParams());
        filters.erase(filters.begin() + fltIdx);
        fillFilters(conf, fltIdx < filters.size() ? fltIdx : fltIdx - 1);
    }
}

void TemplatesConfigDlg::moveFilterUp()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    const int fltIdx = m_lstFilters->currentRow();
    if (conf->hasFilterParam(fltIdx) && (fltIdx > 0))
    {
        auto &filters(conf->getFilterParams());
        std::swap(filters[fltIdx], filters[fltIdx - 1]);
        fillFilters(conf, fltIdx - 1);
    }
}

void TemplatesConfigDlg::moveFilterDown()
{
    auto conf = getCuttentTempl();
    if (conf->isNull())
        return;

    const int fltIdx = m_lstFilters->currentRow();
    auto &filters(conf->getFilterParams());
    if (conf->hasFilterParam(fltIdx) && (fltIdx < filters.size() - 1))
    {
        std::swap(filters[fltIdx], filters[fltIdx + 1]);
        fillFilters(conf, fltIdx + 1);
    }
}

void TemplatesConfigDlg::createActions()
{
    m_actDeleteTempl = new QAction(Style::getIcon("delete_icon.png"), tr("Delete template"), this);
    m_actRunRegex = new QAction(Style::getIcon("exec_icon.png"), tr("Create columns from regex"), this);

    // Columns
    m_actAddColumn = new QAction(Style::getIcon("add_icon.png"), tr("Add column"), this);
    m_actRmColumn = new QAction(Style::getIcon("remove_icon.png"), tr("Remove column"), this);

    // Highlighters
    m_actAddHighlighter = new QAction(Style::getIcon("add_icon.png"), tr("Add Highlighter"), this);
    m_actRmHighlighter = new QAction(Style::getIcon("remove_icon.png"), tr("Remove Highlighter"), this);
    m_actMoveHighlighterUp = new QAction(Style::getIcon("up_icon.png"), tr("Move Up"), this);
    m_actMoveHighlighterDown = new QAction(Style::getIcon("down_icon.png"), tr("Move Down"), this);
    m_actHltForeColor = new QAction(tr("Color of the text"), this);
    m_actHltBackColor = new QAction(tr("Color of the background"), this);

    // Filters
    m_actAddFilter = new QAction(Style::getIcon("add_icon.png"), tr("Add Filter"), this);
    m_actRmFilter = new QAction(Style::getIcon("remove_icon.png"), tr("Remove Filter"), this);
    m_actMoveFilterUp = new QAction(Style::getIcon("up_icon.png"), tr("Move Up"), this);
    m_actMoveFilterDown = new QAction(Style::getIcon("down_icon.png"), tr("Move Down"), this);
}

void TemplatesConfigDlg::createConnections()
{
    connect(
        m_cmbTemplates,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &TemplatesConfigDlg::setCurrentTemplate);
    connect(m_actDeleteTempl, &QAction::triggered, this, &TemplatesConfigDlg::deleteTemplate);
    connect(m_actRunRegex, &QAction::triggered, this, &TemplatesConfigDlg::createColumnsFromRegex);
    connect(m_edtConfName, &QLineEdit::editingFinished, this, &TemplatesConfigDlg::updateTemplateMainInfo);
    connect(m_edtRegex, &QLineEdit::editingFinished, this, &TemplatesConfigDlg::updateTemplateMainInfo);

    // Columns
    connect(
        m_lstColumns,
        QOverload<int>::of(&QListWidget::currentRowChanged),
        this,
        &TemplatesConfigDlg::setCurrentColumn);
    connect(
        m_cmbColType,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &TemplatesConfigDlg::setColumnType);
    connect(m_actAddColumn, &QAction::triggered, this, &TemplatesConfigDlg::addColumn);
    connect(m_actRmColumn, &QAction::triggered, this, &TemplatesConfigDlg::rmColumn);
    connect(m_edtColName, &QLineEdit::editingFinished, this, &TemplatesConfigDlg::updateTemplateColumns);
    connect(m_edtColKey, &QLineEdit::editingFinished, this, &TemplatesConfigDlg::updateTemplateColumns);
    connect(m_cmbColType, QOverload<int>::of(&QComboBox::activated), this, &TemplatesConfigDlg::updateTemplateColumns);
    connect(m_edtColFormat, &QLineEdit::editingFinished, this, &TemplatesConfigDlg::updateTemplateColumns);
    connect(m_chkNoMatchCol, &QCheckBox::stateChanged, this, &TemplatesConfigDlg::updateTemplateColumns);

    // Highlighters
    connect(
        m_lstHighlighters,
        QOverload<int>::of(&QListWidget::currentRowChanged),
        this,
        &TemplatesConfigDlg::setCurrentHighlighter);
    connect(m_actHltForeColor, &QAction::triggered, this, &TemplatesConfigDlg::pickColor);
    connect(m_actHltBackColor, &QAction::triggered, this, &TemplatesConfigDlg::pickColor);
    connect(m_hltSearchCtrl, &SearchParamControl::paramChanged, this, &TemplatesConfigDlg::updateTemplateHighlighters);
    connect(m_actAddHighlighter, &QAction::triggered, this, &TemplatesConfigDlg::addHighlighter);
    connect(m_actRmHighlighter, &QAction::triggered, this, &TemplatesConfigDlg::rmHighlighter);
    connect(m_actMoveHighlighterUp, &QAction::triggered, this, &TemplatesConfigDlg::moveHighlighterUp);
    connect(m_actMoveHighlighterDown, &QAction::triggered, this, &TemplatesConfigDlg::moveHighlighterDown);

    // Filters
    connect(
        m_lstFilters,
        QOverload<int>::of(&QListWidget::currentRowChanged),
        this,
        &TemplatesConfigDlg::setCurrentFilter);
    connect(m_edtFltName, &QLineEdit::editingFinished, this, &TemplatesConfigDlg::updateTemplateFilter);
    connect(m_fltSearchCtrl, &SearchParamControl::paramChanged, this, &TemplatesConfigDlg::updateTemplateFilter);
    connect(m_actAddFilter, &QAction::triggered, this, &TemplatesConfigDlg::addFilter);
    connect(m_actRmFilter, &QAction::triggered, this, &TemplatesConfigDlg::rmFilter);
    connect(m_actMoveFilterUp, &QAction::triggered, this, &TemplatesConfigDlg::moveFilterUp);
    connect(m_actMoveFilterDown, &QAction::triggered, this, &TemplatesConfigDlg::moveFilterDown);

    // Dialog buttons
    connect(m_btnApply, &QPushButton::clicked, this, &TemplatesConfigDlg::apply);
    connect(m_btnSave, &QPushButton::clicked, this, &TemplatesConfigDlg::save);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void TemplatesConfigDlg::configureRegexMode(const FileConf::Ptr &conf)
{
    if (conf->getFileType() == tp::FileType::Text)
    {
        if (!m_edtRegex->isVisible())
        {
            m_edtRegex->setVisible(true);
            m_labRegex->setVisible(true);
            m_btnRunRegex->setVisible(true);
            m_frmTemplMain->addRow(m_labRegex, m_hRegex);
            m_chkNoMatchCol->setVisible(true);
            m_frmColumn->addRow(m_chkNoMatchCol);
            m_edtColKey->setValidator(m_rxGroupValidator);
        }
        m_labColKey->setText(tr("Group"));
        m_edtColKey->setPlaceholderText(tr("Regex capturing group"));
        m_edtRegex->setText(conf->getRegexPattern().c_str());
    }
    else
    {
        if (m_edtRegex->isVisible() && m_frmTemplMain->rowCount() > 0)
        {
            m_edtRegex->setVisible(false);
            m_labRegex->setVisible(false);
            m_btnRunRegex->setVisible(false);
            m_frmTemplMain->takeRow(m_frmTemplMain->rowCount() - 1);
            m_chkNoMatchCol->setVisible(false);
            m_frmColumn->takeRow(m_frmColumn->rowCount() - 1);
            m_edtColKey->setValidator(nullptr);
        }
        m_labColKey->setText(tr("Key"));
        m_edtColKey->setPlaceholderText(tr("Json Key"));
        m_edtRegex->setText(QString());
    }
}

void TemplatesConfigDlg::buildLayout()
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

    m_rxGroupValidator = new QIntValidator(this);

    // Root layout -------------------------------------------------------------------------------- (Start)
    auto vLayoutMain = new QVBoxLayout(this);

    // Template selection and delete
    m_cmbTemplates = new QComboBox(this);
    auto btnDeleteTempl = new QToolButton(this);
    btnDeleteTempl->setDefaultAction(m_actDeleteTempl);
    auto hLayoutTemplate = new QHBoxLayout();
    hLayoutTemplate->addWidget(m_cmbTemplates);
    hLayoutTemplate->addWidget(btnDeleteTempl);
    hLayoutTemplate->setAlignment(Qt::AlignTop);
    vLayoutMain->addLayout(hLayoutTemplate);

    // Create template frame ------------------------------------------------------------ (Start)
    m_frameTempl = new QFrame(this);
    auto vTemplFrameMain = new QVBoxLayout(m_frameTempl);

    // Template edit form ----------------------------------------------------- (Start)
    m_frmTemplMain = new QFormLayout();

    // File type
    m_edtFileType = new QLineEdit(m_frameTempl);
    auto edtFileTypePalette = m_edtFileType->palette();
    edtFileTypePalette.setColor(QPalette::Disabled, QPalette::Base, edtFileTypePalette.color(QPalette::Window));
    edtFileTypePalette.setColor(QPalette::Disabled, QPalette::Text, edtFileTypePalette.color(QPalette::WindowText));
    m_edtFileType->setPalette(edtFileTypePalette);
    m_edtFileType->setEnabled(false);
    m_frmTemplMain->addRow(tr("Type"), m_edtFileType);

    // Template name
    m_edtConfName = new QLineEdit(m_frameTempl);
    m_edtConfName->setPlaceholderText(tr("Template name"));
    m_frmTemplMain->addRow(tr("Name"), m_edtConfName);

    // Regular expression
    // Will be added or removed according to the type of the file.
    m_labRegex = new QLabel(tr("Regex"), m_frameTempl);
    m_labRegex->setVisible(false);
    m_edtRegex = new QLineEdit(m_frameTempl);
    m_edtRegex->setPlaceholderText(tr("Regular expression to define the columns"));
    m_edtRegex->setVisible(false);
    m_btnRunRegex = new QToolButton(m_frameTempl);
    m_btnRunRegex->setDefaultAction(m_actRunRegex);
    m_btnRunRegex->setVisible(false);
    m_hRegex = new QHBoxLayout();
    m_hRegex->addWidget(m_edtRegex);
    m_hRegex->addWidget(m_btnRunRegex);

    vTemplFrameMain->addLayout(m_frmTemplMain);
    // Template edit form ----------------------------------------------------- (End)

    m_tabWidgets = new QTabWidget(m_frameTempl);

    // Create columns tab ----------------------------------------------------- (Start)
    m_tabColumns = new QWidget();
    auto hColumnsTab = new QHBoxLayout(m_tabColumns);

    // Columns list ------------------------------------------------- (Start)
    auto vColumns = new QVBoxLayout();
    m_lstColumns = new QListWidget(m_tabColumns);
    vColumns->addWidget(m_lstColumns);

    auto hColumnsLstButtons = new QHBoxLayout();

    auto btnAddColumn = new QToolButton(m_tabColumns);
    btnAddColumn->setDefaultAction(m_actAddColumn);
    hColumnsLstButtons->addWidget(btnAddColumn);

    auto btnRmColumn = new QToolButton(m_tabColumns);
    btnRmColumn->setDefaultAction(m_actRmColumn);
    hColumnsLstButtons->addWidget(btnRmColumn);

    hColumnsLstButtons->addStretch();
    vColumns->addLayout(hColumnsLstButtons);

    hColumnsTab->addLayout(vColumns);
    // Columns list ------------------------------------------------- (End)

    // Column edit form --------------------------------------------- (Start)
    m_frmColumn = new QFormLayout();
    m_edtColName = new QLineEdit(m_tabColumns);
    m_edtColName->setPlaceholderText(tr("Column display name"));
    m_frmColumn->addRow(tr("Name"), m_edtColName);

    m_labColKey = new QLabel(QString(), m_tabColumns);
    m_edtColKey = new QLineEdit(m_tabColumns);
    m_frmColumn->addRow(m_labColKey, m_edtColKey);

    m_cmbColType = new QComboBox(m_tabColumns);
    auto cmbColTypeSizePolicy = m_cmbColType->sizePolicy();
    cmbColTypeSizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_cmbColType->setSizePolicy(cmbColTypeSizePolicy);
    m_frmColumn->addRow(tr("Type"), m_cmbColType);

    m_edtColFormat = new QLineEdit(m_tabColumns);
    m_edtColFormat->setPlaceholderText(tr("Date Time Format"));
    m_frmColumn->addRow(tr("Format"), m_edtColFormat);

    // Will be added or removed according to the type of the file.
    m_chkNoMatchCol = new QCheckBox(tr("Use this column for no matches"), m_tabColumns);
    m_chkNoMatchCol->setVisible(false);

    hColumnsTab->addLayout(m_frmColumn);
    // Column edit form --------------------------------------------- (Start)

    m_tabWidgets->addTab(m_tabColumns, tr("Columns"));
    // Create columns tab ----------------------------------------------------- (End)

    // Create highlighters tab ------------------------------------------------ (Start)
    m_tabHighlighters = new QWidget();
    auto hHighlightersTab = new QHBoxLayout(m_tabHighlighters);

    // Highlighters list -------------------------------------------- (Start)
    auto vHighlighters = new QVBoxLayout();
    m_lstHighlighters = new QListWidget(m_tabHighlighters);
    vHighlighters->addWidget(m_lstHighlighters);

    auto hHighlightersLstButtons = new QHBoxLayout();
    auto btnAddHighlighter = new QToolButton(m_tabHighlighters);
    btnAddHighlighter->setDefaultAction(m_actAddHighlighter);
    hHighlightersLstButtons->addWidget(btnAddHighlighter);

    auto btnRmHighlighter = new QToolButton(m_tabHighlighters);
    btnRmHighlighter->setDefaultAction(m_actRmHighlighter);
    hHighlightersLstButtons->addWidget(btnRmHighlighter);

    hHighlightersLstButtons->addStretch();

    auto btnMoveHighlighterUp = new QToolButton(m_tabHighlighters);
    btnMoveHighlighterUp->setDefaultAction(m_actMoveHighlighterUp);
    hHighlightersLstButtons->addWidget(btnMoveHighlighterUp);

    auto btnMoveHighlighterDown = new QToolButton(m_tabHighlighters);
    btnMoveHighlighterDown->setDefaultAction(m_actMoveHighlighterDown);
    hHighlightersLstButtons->addWidget(btnMoveHighlighterDown);

    vHighlighters->addLayout(hHighlightersLstButtons);

    hHighlightersTab->addLayout(vHighlighters);
    // Highlighters list -------------------------------------------- (End)

    // Highlighter edit form ---------------------------------------- (Start)
    auto frmHighlighter = new QFormLayout();

    m_cmbHltColumn = new QComboBox(m_tabHighlighters);
    auto cmbHltColumnSizePolicy = m_cmbHltColumn->sizePolicy();
    cmbHltColumnSizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_cmbHltColumn->setSizePolicy(cmbHltColumnSizePolicy);
    frmHighlighter->addRow(tr("Column"), m_cmbHltColumn);

    m_edtHltPattern = new QLineEdit(m_tabHighlighters);
    frmHighlighter->addRow(tr("Pattern"), m_edtHltPattern);

    m_hltSearchCtrl = new SearchParamControl(m_cmbHltColumn, m_edtHltPattern, m_tabHighlighters);
    frmHighlighter->addRow(tr("Options"), m_hltSearchCtrl);

    auto btnHltForeColor = new QToolButton(m_tabHighlighters);
    btnHltForeColor->setDefaultAction(m_actHltForeColor);
    frmHighlighter->addRow(tr("Fore Color"), btnHltForeColor);

    auto btnHltBackColor = new QToolButton(m_tabHighlighters);
    btnHltBackColor->setDefaultAction(m_actHltBackColor);
    frmHighlighter->addRow(tr("Back Color"), btnHltBackColor);

    hHighlightersTab->addLayout(frmHighlighter);
    // Highlighter edit form ---------------------------------------- (End)

    m_tabWidgets->addTab(m_tabHighlighters, tr("Highlighters"));
    // Create highlighters tab ------------------------------------------------ (End)

    // Create filters tab ----------------------------------------------------- (Start)
    m_tabFilters = new QWidget();
    auto hFiltersTab = new QHBoxLayout(m_tabFilters);

    // Filters list ------------------------------------------------- (Start)
    auto vFilters = new QVBoxLayout();
    m_lstFilters = new QListWidget(m_tabFilters);
    vFilters->addWidget(m_lstFilters);

    auto hFiltersLstButtons = new QHBoxLayout();
    auto btnAddFilter = new QToolButton(m_tabFilters);
    btnAddFilter->setDefaultAction(m_actAddFilter);
    hFiltersLstButtons->addWidget(btnAddFilter);

    auto btnRmFilter = new QToolButton(m_tabFilters);
    btnRmFilter->setDefaultAction(m_actRmFilter);
    hFiltersLstButtons->addWidget(btnRmFilter);

    hFiltersLstButtons->addStretch();

    auto btnMoveFilterUp = new QToolButton(m_tabFilters);
    btnMoveFilterUp->setDefaultAction(m_actMoveFilterUp);
    hFiltersLstButtons->addWidget(btnMoveFilterUp);

    auto btnMoveFilterDown = new QToolButton(m_tabFilters);
    btnMoveFilterDown->setDefaultAction(m_actMoveFilterDown);
    hFiltersLstButtons->addWidget(btnMoveFilterDown);

    vFilters->addLayout(hFiltersLstButtons);

    hFiltersTab->addLayout(vFilters);
    // Filters list ------------------------------------------------- (End)

    // Filters edit form -------------------------------------------- (Start)
    auto frmFilter = new QFormLayout();

    m_edtFltName = new QLineEdit(m_tabFilters);
    frmFilter->addRow(tr("Name"), m_edtFltName);

    m_cmbFltColumn = new QComboBox(m_tabFilters);
    auto cmbFltColumnSizePolicy = m_cmbFltColumn->sizePolicy();
    cmbFltColumnSizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_cmbFltColumn->setSizePolicy(cmbFltColumnSizePolicy);
    frmFilter->addRow(tr("Column"), m_cmbFltColumn);

    m_edtFltPattern = new QLineEdit(m_tabFilters);
    frmFilter->addRow(tr("Pattern"), m_edtFltPattern);

    m_fltSearchCtrl = new SearchParamControl(m_cmbFltColumn, m_edtFltPattern, m_tabFilters);
    frmFilter->addRow(tr("Options"), m_fltSearchCtrl);

    hFiltersTab->addLayout(frmFilter);
    // Filters edit form -------------------------------------------- (End)

    m_tabWidgets->addTab(m_tabFilters, tr("Filters"));
    // Create filters tab ----------------------------------------------------- (End)

    vTemplFrameMain->addWidget(m_tabWidgets);
    vLayoutMain->addWidget(m_frameTempl);
    // Create template frame ------------------------------------------------------------ (End)

    // Create dialog Buttons -------------------------------------------------- (Start)
    auto hDlgButtons = new QHBoxLayout();

    m_btnApply = new QPushButton(tr("&Apply"), this);
    m_btnApply->setAutoDefault(false);
    hDlgButtons->addWidget(m_btnApply);

    m_btnSave = new QPushButton(tr("&Save"), this);
    m_btnSave->setAutoDefault(false);
    hDlgButtons->addWidget(m_btnSave);

    hDlgButtons->addStretch();

    m_btnCancel = new QPushButton(tr("&Cancel"), this);
    m_btnCancel->setAutoDefault(false);

    hDlgButtons->addWidget(m_btnCancel);
    // Create dialog Buttons -------------------------------------------------- (Start)

    vLayoutMain->addLayout(hDlgButtons);
    // Root layout -------------------------------------------------------------------------------- (End)
}
