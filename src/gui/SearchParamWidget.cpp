// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "SearchParamWidget.h"
#include "SearchParamControl.h"
#include "SearchParamModel.h"
#include "Style.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QLineEdit>
#include <QToolButton>
#include <QComboBox>
#include <QStringListModel>

SearchParamWidget::SearchParamWidget(FileConf::Ptr conf, SearchParamModel *searchModel, QWidget *parent) : QWidget(parent)
{
    createActions();

    m_cmbColumns = new QComboBox(this);

    m_cmbSearch = new QComboBox(this);
    auto cmbSearchSizePolicy = m_cmbSearch->sizePolicy();
    cmbSearchSizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_cmbSearch->setSizePolicy(cmbSearchSizePolicy);
    m_cmbSearch->setFocusPolicy(Qt::StrongFocus);

    m_control = new SearchParamControl(m_cmbColumns, m_cmbSearch, searchModel, this);
    m_control->setFileConf(conf);

    QToolButton *btnDisableMe = new QToolButton(this);
    btnDisableMe->setFocusPolicy(Qt::NoFocus);
    btnDisableMe->setDefaultAction(m_actDisableMe);

    QToolButton *btnRemoveMe = new QToolButton(this);
    btnRemoveMe->setFocusPolicy(Qt::NoFocus);
    btnRemoveMe->setDefaultAction(m_actRemoveMe);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(m_cmbColumns);
    hLayout->addWidget(m_control);
    hLayout->addWidget(m_cmbSearch);
    hLayout->addWidget(btnDisableMe);
    hLayout->addWidget(btnRemoveMe);

    translateUi();

    setLayout(hLayout);

    createConnections();
}

SearchParamWidget::~SearchParamWidget()
{
}

void SearchParamWidget::createActions()
{
    m_actDisableMe = new QAction(this);
    m_actDisableMe->setCheckable(true);

    m_actRemoveMe = new QAction(this);
}

void SearchParamWidget::translateUi()
{
    m_actDisableMe->setText(tr("Disable Parameter"));
    m_actDisableMe->setIcon(Style::getIcon("filter_off_icon.png"));

    m_actRemoveMe->setText(tr("Remove Parameter"));
    m_actRemoveMe->setIcon(Style::getIcon("delete_icon.png"));
}

void SearchParamWidget::retranslateUi()
{
    translateUi();
    m_control->retranslateUi();
}

void SearchParamWidget::createConnections()
{
    connect(m_control, &SearchParamControl::searchRequested, this, &SearchParamWidget::searchRequested);
    connect(m_actRemoveMe, &QAction::triggered, this, [this]() { emit deleteRequested(this); });
}

void SearchParamWidget::apply()
{
    m_control->apply();
}

void SearchParamWidget::reconfigure()
{
    m_control->reconfigure();
}

bool SearchParamWidget::getIsEnabled() const
{
    return !m_actDisableMe->isChecked();
}

tp::SearchParam SearchParamWidget::getSearchParam() const
{
    return m_control->getSearchParam();
}
