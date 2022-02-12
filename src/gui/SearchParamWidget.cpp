// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "SearchParamWidget.h"
#include "SearchParamControl.h"
#include "Style.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QLineEdit>
#include <QToolButton>
#include <QComboBox>

SearchParamWidget::SearchParamWidget(FileConf::Ptr conf, QWidget *parent) : QWidget(parent)
{
    createActions();

    m_cmbColumns = new QComboBox(this);

    m_txtSearch = new QLineEdit(this);
    m_txtSearch->setFocusPolicy(Qt::StrongFocus);

    m_control = new SearchParamControl(m_cmbColumns, m_txtSearch);
    m_control->setFileConf(conf);

    QToolButton *btnDisableMe = new QToolButton(this);
    btnDisableMe->setFocusPolicy(Qt::NoFocus);
    btnDisableMe->setDefaultAction(m_actDisableMe);

    QToolButton *btnRemoveMe = new QToolButton(this);
    btnRemoveMe->setFocusPolicy(Qt::NoFocus);
    btnRemoveMe->setDefaultAction(m_actRemoveMe);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->addWidget(m_cmbColumns);
    hLayout->addWidget(m_control);
    hLayout->addWidget(m_txtSearch);
    hLayout->addWidget(btnDisableMe);
    hLayout->addWidget(btnRemoveMe);

    setLayout(hLayout);

    createConnections();
}

SearchParamWidget::~SearchParamWidget()
{
}

void SearchParamWidget::createActions()
{
    m_actDisableMe = new QAction(tr("Disable Parameter"), this);
    m_actDisableMe->setIcon(Style::getIcon("filter_off_icon.png"));
    m_actDisableMe->setCheckable(true);

    m_actRemoveMe = new QAction(tr("Remove Parameter"), this);
    m_actRemoveMe->setIcon(Style::getIcon("delete_icon.png"));
}

void SearchParamWidget::createConnections()
{
    connect(m_control, &SearchParamControl::searchRequested, this, &SearchParamWidget::searchRequested);
    connect(m_actRemoveMe, &QAction::triggered, this, [this]() { emit deleteRequested(this); });
}

void SearchParamWidget::updateColumns()
{
    m_control->updateColumns(true);
}

bool SearchParamWidget::getIsEnabled() const
{
    return !m_actDisableMe->isChecked();
}

tp::SearchParam SearchParamWidget::getSearchParam() const
{
    return m_control->getSearchParam();
}
