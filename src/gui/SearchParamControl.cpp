// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "SearchParamControl.h"
#include "LogViewWidget.h"
#include "TextLogModel.h"
#include "JsonLogModel.h"
#include "ProxyModel.h"
#include "Style.h"
#include "LongScrollBar.h"
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QAction>
#include <QToolBar>
#include <QMenuBar>
#include <QLineEdit>
#include <QToolButton>
#include <QComboBox>

#include <fstream>

SearchParamControl::SearchParamControl(QComboBox *cmbColumns, QLineEdit *edtPattern, QWidget *parent)
    : QWidget(parent),
      m_cmbColumns(cmbColumns),
      m_edtPattern(edtPattern)
{
    createActions();

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
    hLayout->addWidget(m_cmbColumns);
    hLayout->addWidget(btnMatchCase);
    hLayout->addWidget(btnRegex);
    hLayout->addWidget(btnRange);
    hLayout->addWidget(btnNotOp);

    setLayout(hLayout);
    createConnections();
    updateParam(false);
    updateColumns(false);
}

SearchParamControl::~SearchParamControl()
{
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
    connect(
        m_edtPattern,
        &QLineEdit::returnPressed,
        this,
        [this]()
        {
            updateParam();
            emit searchRequested();
        });
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
    m_edtPattern->setText(param.pattern.c_str());
    m_actRange->setChecked(param.type == tp::SearchType::Range);
    m_actRegex->setChecked(param.type == tp::SearchType::Regex);
    m_actMatchCase->setChecked(param.flags.has(tp::SearchFlag::MatchCase));
    m_actNotOp->setChecked(param.flags.has(tp::SearchFlag::NotOperator));

    int cmbIndex(0);
    if (param.column.has_value())
    {
        const auto colIdx = param.column.value().idx;
        if (m_conf && !m_conf->isNull() && m_conf->hasDefinedColumn(colIdx))
        {
            cmbIndex = colIdx + 1;
        }
    }
    m_cmbColumns->setCurrentIndex(cmbIndex);
    updateParam(notifyChanged);
}