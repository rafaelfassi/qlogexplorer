// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "SearchParamControl.h"
#include "SearchParamModel.h"
#include "TemplatesConfigDlg.h"
#include "Style.h"
#include "Settings.h"
#include <QHBoxLayout>
#include <QAction>
#include <QLineEdit>
#include <QToolButton>
#include <QComboBox>
#include <QCompleter>
#include <QMenu>
#include <QContextMenuEvent>
#include <QSortFilterProxyModel>

class SearchLineEdit : public QLineEdit
{
public:
    using QLineEdit::QLineEdit;
    void setControl(SearchParamControl *control) { m_control = control; }

protected:
    void contextMenuEvent(QContextMenuEvent *event) override
    {
        QMenu *menu = createStandardContextMenu();
        const auto currText = utl::toStr(text());
        if (!currText.empty() && m_control && (m_control->getSearchParam().pattern == currText))
        {
            menu->addSeparator();
            auto act = menu->addAction(tr("Add to Template"));
            connect(act, &QAction::triggered, this, &SearchLineEdit::addToTemplate);
        }

        menu->exec(event->globalPos());
        delete menu;
    }

private slots:
    void addToTemplate()
    {
        if (!m_control)
            return;

        TemplatesConfigDlg dlg;
        tp::FilterParam flt;
        flt.searchParam = m_control->getSearchParam();
        dlg.setOpenActionAddFilter(flt);
        dlg.exec();
    }

private:
    SearchParamControl *m_control = nullptr;
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
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setAlignment(Qt::AlignLeft);
    hLayout->addWidget(m_cmbColumns);
    hLayout->addWidget(btnMatchCase);
    hLayout->addWidget(btnRegex);
    hLayout->addWidget(btnRange);
    hLayout->addWidget(btnNotOp);

    auto pol = sizePolicy();
    pol.setHorizontalPolicy(QSizePolicy::Minimum);
    setSizePolicy(pol);

    translateUi();

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
    : SearchParamControl(
          cmbColumns,
          [cmbSearch]()
          {
              cmbSearch->setEditable(true);
              auto lineEdit = new SearchLineEdit(nullptr);
              cmbSearch->setLineEdit(lineEdit);
              return lineEdit;
          }(),
          parent)
{
    m_proxyModel = searchModel->newProxy(this, std::bind(&SearchParamControl::getSearchParam, this));
    m_cmbSearch = cmbSearch;

    // Workaround
    // The QComboBox placeholder is not visible when it's editable, because in this case it'll
    // consider the placeholder of the QLineEdit instead.
    // But when the QComboBox placeholder is empty, it'll set the current index to 0 when the
    // current index is -1 and a new item is added.
    // See QComboBoxPrivate::_q_rowsInserted for further info.
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    m_cmbSearch->setPlaceholderText(m_edtPattern->placeholderText());
#endif

    m_cmbSearch->setModel(m_proxyModel);
    m_cmbSearch->setInsertPolicy(QComboBox::NoInsert);
    m_cmbSearch->setCurrentIndex(-1);
    auto lineEdit = static_cast<SearchLineEdit *>(m_cmbSearch->lineEdit());
    if (lineEdit)
    {
        lineEdit->setParent(m_cmbSearch);
        lineEdit->setControl(this);
    }

    m_completer = m_cmbSearch->completer();
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCompletionRole(Qt::DisplayRole);

    connect(
        m_cmbSearch,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &SearchParamControl::cmbSearchCurrentIndexChanged);
}

void SearchParamControl::cmbSearchCurrentIndexChanged(int idx)
{
    LOG_INF("cmbSearch - IndexChanged {}", idx);
    if (m_proxyModel != nullptr)
    {
        if (m_proxyModel->isReady())
        {
            const auto &searchParam = m_proxyModel->setCurrentItemIdx(idx);
            if (searchParam.has_value())
            {
                setSearchParam(searchParam.value());
            }
        }
        else if ((m_cmbSearch != nullptr) && (m_cmbSearch->currentIndex() != -1))
        {
            LOG_INF("cmbSearch - Model is not ready yet");
            m_cmbSearch->setCurrentIndex(-1);
        }
    }
}

void SearchParamControl::apply()
{
    updateParam();

    if (m_cmbSearch != nullptr && m_proxyModel != nullptr)
    {
        if (!m_cmbSearch->currentText().isEmpty() && m_proxyModel->isReady())
        {
            if (m_cmbSearch->findData(m_cmbSearch->currentText(), Qt::EditRole) == -1)
            {
                // If the pattern dont exists, needs to be added
                m_cmbSearch->addItem(m_cmbSearch->currentText());
                m_cmbSearch->setCurrentIndex(m_cmbSearch->count() - 1);
            }
            else if (const auto &searchParam = m_proxyModel->getCurrentItemParam(); searchParam.has_value())
            {
                const auto &currSearchParam = getSearchParam();
                if (!tp::areSimilar(searchParam.value(), currSearchParam))
                {
                    // The search params are not the same and needs to be updated
                    const auto idx =
                        m_cmbSearch->findData(m_cmbSearch->currentText(), ParamModelRoles::NotPredefinedParam);
                    if (idx == -1)
                    {
                        // There are only the predefined parameter
                        // Cannot update it, so adding a not predefined one
                        m_cmbSearch->addItem(m_cmbSearch->currentText());
                        m_cmbSearch->setCurrentIndex(m_cmbSearch->count() - 1);
                    }
                    else
                    {
                        // Updates the search param
                        m_proxyModel->setSearchParam(idx, currSearchParam);
                        if (idx != m_cmbSearch->currentIndex())
                        {
                            // A predefined parameter is selected, but there is a not predefined one with the same
                            // pattern.
                            // Selects the not predefined one.
                            m_cmbSearch->setCurrentIndex(idx);
                        }
                    }
                }
            }
        }
        else if (m_cmbSearch->currentIndex() != -1)
        {
            // The m_cmbSearch text is empty or its model is not ready
            m_cmbSearch->setCurrentIndex(-1);
        }
    }

    if (m_proxyModel != nullptr)
    {
        m_proxyModel->applyCurrentItem();
    }
}

void SearchParamControl::createActions()
{
    m_actRegex = new QAction(this);
    m_actRegex->setCheckable(true);
    m_actRegex->setChecked(Settings::getDefaultSearchType() == tp::SearchType::Regex);

    m_actRange = new QAction(this);
    m_actRange->setCheckable(true);

    m_actMatchCase = new QAction(this);
    m_actMatchCase->setCheckable(true);

    m_actNotOp = new QAction(this);
    m_actNotOp->setCheckable(true);
}

void SearchParamControl::translateUi()
{
    m_actRegex->setText(tr("Regular Expression"));
    m_actRegex->setIcon(Style::getIcon("regex_icon.png"));

    m_actRange->setText(tr("Range match (from -> to)"));
    m_actRange->setIcon(Style::getIcon("range_icon.png"));

    m_actMatchCase->setText(tr("Match Case"));
    m_actMatchCase->setIcon(Style::getIcon("case_sensitive_icon.png"));

    m_actNotOp->setText(tr("Not (invert the match)"));
    m_actNotOp->setIcon(Style::getIcon("not_icon.png"));

    m_edtPattern->setPlaceholderText(tr("Pattern"));
}

void SearchParamControl::retranslateUi()
{
    translateUi();
    Style::updateWidget(m_cmbColumns);
    Style::updateWidget(m_edtPattern);
    if (m_cmbSearch != nullptr)
        Style::updateWidget(m_cmbSearch);
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

void SearchParamControl::reconfigure()
{
    updateColumns(true);
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
