// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "SearchParamControl.h"
#include "SearchParamModel.h"
#include "Style.h"
#include <QHBoxLayout>
#include <QAction>
#include <QLineEdit>
#include <QToolButton>
#include <QComboBox>
#include <QCompleter>
#include <QIdentityProxyModel>

class SearchParamProxyModel : public QIdentityProxyModel
{
public:
    SearchParamProxyModel(SearchParamModel *model, SearchParamControl *control)
        : QIdentityProxyModel(control),
          m_model(model),
          m_control(control)
    {
        setSourceModel(m_model);
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        if (index.row() >= 0 && index.row() < m_model->rowCount() && (role == Qt::EditRole || role == Qt::DisplayRole))
        {
            const QString valueString = value.toString();
            auto &param = m_model->getRowData(index.row());
            if (m_model->matchRowData(valueString, param))
                return true;

            param.name = utl::toStr(valueString);
            param.searchParam = m_control->getSearchParam();
            param.searchParam.pattern = param.name;
            emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
            return true;
        }
        return false;
    }

    void setCurrentItemIdx(int idx)
    {
        if (!m_model->isValidIdx(idx))
            return;

        const auto &currentText = m_model->getItemName(idx);
        if (m_currentText != currentText)
        {
            m_currentText = currentText;
            m_control->setSearchParam(m_model->getSearchParam(idx));
        }
    }

    void applyCurrentItem()
    {
        const auto idx = m_model->findByItemName(m_currentText);
        if (m_model->isValidIdx(idx))
        {
            m_model->setSearchParam(idx, m_control->getSearchParam());
            m_model->moveRow(QModelIndex(), idx, QModelIndex(), 0);
            setCurrentItemIdx(0);
        }
    }

    int getCurrentIdx() { return m_model->findByItemName(m_currentText); }

private:
    SearchParamModel *m_model;
    SearchParamControl *m_control;
    QString m_currentText;
};

SearchParamControl::SearchParamControl(QComboBox *cmbColumns, QLineEdit *edtPattern, QWidget *parent)
    : QWidget(parent),
      m_cmbColumns(cmbColumns),
      m_edtPattern(edtPattern)
{
    Q_ASSERT_X(cmbColumns != nullptr, "SearchParamControl", "cmbColumns is null");
    Q_ASSERT_X(edtPattern != nullptr, "SearchParamControl", "edtPattern is null");

    createActions();

    cmbColumns->setFocusPolicy(Qt::NoFocus);

    QToolButton *btnMatchCase = new QToolButton(this);
    btnMatchCase->setFocusPolicy(Qt::NoFocus);
    btnMatchCase->setDefaultAction(m_actMatchCase);

    QToolButton *btnRegex = new QToolButton(this);
    btnRegex->setFocusPolicy(Qt::NoFocus);
    btnRegex->setDefaultAction(m_actRegex);

    QToolButton *btnRange = new QToolButton(this);
    btnRange->setFocusPolicy(Qt::NoFocus);
    btnRange->setDefaultAction(m_actRange);

    QToolButton *btnNotOp = new QToolButton(this);
    btnNotOp->setFocusPolicy(Qt::NoFocus);
    btnNotOp->setDefaultAction(m_actNotOp);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setAlignment(Qt::AlignLeft);
    hLayout->addWidget(m_cmbColumns);
    hLayout->addWidget(btnMatchCase);
    hLayout->addWidget(btnRegex);
    hLayout->addWidget(btnRange);
    hLayout->addWidget(btnNotOp);

    auto pol = sizePolicy();
    pol.setHorizontalPolicy(QSizePolicy::Minimum);
    setSizePolicy(pol);

    setLayout(hLayout);
    createConnections();
    updateParam(false);
    updateColumns(false);
}

SearchParamControl::SearchParamControl(
    QComboBox *cmbColumns,
    QComboBox *cmbSearch,
    SearchParamModel *searchModel,
    QWidget *parent)
    : SearchParamControl(cmbColumns, cmbSearch->lineEdit(), parent)
{
    Q_ASSERT_X(cmbSearch->isEditable(), "SearchParamControl", "cmbSearch not editable");

    m_proxyModel = new SearchParamProxyModel(searchModel, this);
    m_cmbSearch = cmbSearch;
    m_cmbSearch->setModel(m_proxyModel);
    m_cmbSearch->setInsertPolicy(QComboBox::InsertAtTop);
    m_cmbSearch->setCurrentIndex(-1);

    m_completer = m_cmbSearch->completer();
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCompletionRole(Qt::DisplayRole);

    connect(
        m_cmbSearch,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        m_proxyModel,
        &SearchParamProxyModel::setCurrentItemIdx);
}

void SearchParamControl::apply()
{
    updateParam();
    if (m_proxyModel != nullptr)
    {
        m_proxyModel->applyCurrentItem();
        if (m_completer != nullptr)
        {
            // Needs to reset the the Completer model to get the items in the right order after the current item was
            // moved to the top. It should use QSortFilterProxyModel, but it didn't work well when adding new items
            // after sorting. Verify if it's working fine in future Qt versions.
            m_completer->setModel(m_proxyModel);
        }
    }
}

void SearchParamControl::createActions()
{
    m_actRegex = new QAction(tr("Regular Expression"), this);
    m_actRegex->setIcon(Style::getIcon("regex_icon.png"));
    m_actRegex->setCheckable(true);

    m_actRange = new QAction(tr("Range match (from -> to)"), this);
    m_actRange->setIcon(Style::getIcon("range_icon.png"));
    m_actRange->setCheckable(true);

    m_actMatchCase = new QAction(tr("Match Case"), this);
    m_actMatchCase->setIcon(Style::getIcon("case_sensitive_icon.png"));
    m_actMatchCase->setCheckable(true);

    m_actNotOp = new QAction(tr("Not (invert the match)"), this);
    m_actNotOp->setIcon(Style::getIcon("not_icon.png"));
    m_actNotOp->setCheckable(true);
}

void SearchParamControl::createConnections()
{
    connect(m_actRegex, &QAction::triggered, this, [this]() { updateParam(); });
    connect(m_actRange, &QAction::triggered, this, [this]() { updateParam(); });
    connect(m_actMatchCase, &QAction::triggered, this, [this]() { updateParam(); });
    connect(m_actNotOp, &QAction::triggered, this, [this]() { updateParam(); });
    connect(m_cmbColumns, QOverload<int>::of(&QComboBox::activated), this, [this]() { updateParam(); });
    connect(m_edtPattern, &QLineEdit::editingFinished, this, [this]() { updateParam(); });
    // Must be QueuedConnection due the Completer is not finished when returnPressed is emitted.
    connect(m_edtPattern, &QLineEdit::returnPressed, this, &SearchParamControl::searchRequested, Qt::QueuedConnection);
}

void SearchParamControl::updateColumns(bool tryKeepSel)
{
    const auto oriIdx = m_cmbColumns->currentIndex();
    const auto oriText = m_cmbColumns->currentText();

    m_cmbColumns->clear();
    m_cmbColumns->addItem(tr("All Columns"));

    if (m_conf && !m_conf->isNull() && m_conf->hasDefinedColumns())
    {
        for (const auto &column : m_conf->getColumns())
        {
            m_cmbColumns->addItem(column.name.c_str());
        }
    }

    if (tryKeepSel && (oriIdx >= 0) && (oriIdx < m_cmbColumns->count()) && (oriText == m_cmbColumns->itemText(oriIdx)))
    {
        m_cmbColumns->setCurrentIndex(oriIdx);
    }
    else
    {
        m_cmbColumns->setCurrentIndex(0);
    }

    if (m_cmbSearch != nullptr && m_proxyModel != nullptr)
        m_cmbSearch->setCurrentIndex(m_proxyModel->getCurrentIdx());
}

void SearchParamControl::updateParam(bool notifyChanged)
{
    m_actRange->setEnabled(m_cmbColumns->currentIndex() > 0);
    if (!m_actRange->isEnabled())
        m_actRange->setChecked(false);

    const bool rangeActive(m_actRange->isEnabled() && m_actRange->isChecked());
    m_actMatchCase->setEnabled(!rangeActive);
    m_actRegex->setEnabled(!rangeActive);

    tp::SearchParam param(m_param);

    param.pattern = utl::toStr(m_edtPattern->text());

    if (m_actRange->isEnabled() && m_actRange->isChecked())
        param.type = tp::SearchType::Range;
    else if (m_actRegex->isEnabled() && m_actRegex->isChecked())
        param.type = tp::SearchType::Regex;
    else
        param.type = tp::SearchType::SubString;

    param.flags.reset();
    if (m_actMatchCase->isEnabled() && m_actMatchCase->isChecked())
        param.flags.set(tp::SearchFlag::MatchCase);
    if (m_actNotOp->isEnabled() && m_actNotOp->isChecked())
        param.flags.set(tp::SearchFlag::NotOperator);

    const auto colIdx = m_cmbColumns->currentIndex() - 1;
    if (m_conf && !m_conf->isNull() && m_conf->hasDefinedColumn(colIdx))
    {
        param.column = m_conf->getColumns().at(colIdx);
    }
    else
    {
        param.column = std::nullopt;
    }

    if (param != m_param)
    {
        m_param = param;
        if (notifyChanged)
            emit paramChanged();
    }
}

void SearchParamControl::setFileConf(FileConf::Ptr conf)
{
    m_conf = conf;
    updateColumns(false);
}

tp::SearchParam SearchParamControl::getSearchParam() const
{
    return m_param;
}

void SearchParamControl::setSearchParam(const tp::SearchParam &param, bool notifyChanged)
{
    tp::SearchParam fixedParam(param);
    fixParam(m_conf, fixedParam);

    m_edtPattern->setText(fixedParam.pattern.c_str());
    m_actRange->setChecked(fixedParam.type == tp::SearchType::Range);
    m_actRegex->setChecked(fixedParam.type == tp::SearchType::Regex);
    m_actMatchCase->setChecked(fixedParam.flags.has(tp::SearchFlag::MatchCase));
    m_actNotOp->setChecked(fixedParam.flags.has(tp::SearchFlag::NotOperator));

    int cmbIndex(0);
    if (fixedParam.column.has_value())
    {
        const auto colIdx = fixedParam.column.value().idx;
        if (m_conf && !m_conf->isNull() && m_conf->hasDefinedColumn(colIdx))
        {
            cmbIndex = colIdx + 1;
        }
    }
    m_cmbColumns->setCurrentIndex(cmbIndex);
    updateParam(notifyChanged);
}

void SearchParamControl::fixParam(const FileConf::Ptr &conf, tp::SearchParam &param)
{
    if (param.column.has_value())
    {
        if (!conf || conf->isNull() || !conf->hasDefinedColumn(param.column.value().idx))
        {
            param.column = std::nullopt;
        }
    }
    if ((param.type == tp::SearchType::Range) && !param.column.has_value())
    {
        param.type = tp::SearchType::SubString;
    }
}
