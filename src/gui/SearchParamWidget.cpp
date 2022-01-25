#include "pch.h"
#include "SearchParamWidget.h"
#include "LogViewWidget.h"
#include "TextLogModel.h"
#include "JsonLogModel.h"
#include "ProxyModel.h"
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

SearchParamWidget::SearchParamWidget(AbstractModel *model, QWidget *parent) : QWidget(parent), m_model(model)
{
    createActions();

    m_cmbColumns = new QComboBox(this);
    m_cmbColumns->addItem(tr("All Columns"));

    QToolButton *btnMatchCase = new QToolButton(this);
    btnMatchCase->setFocusPolicy(Qt::NoFocus);
    btnMatchCase->setDefaultAction(m_actMatchCase);

    QToolButton *btnRegex = new QToolButton(this);
    btnRegex->setFocusPolicy(Qt::NoFocus);
    btnRegex->setDefaultAction(m_actRegex);

    QToolButton *btnNotOp = new QToolButton(this);
    btnNotOp->setFocusPolicy(Qt::NoFocus);
    btnNotOp->setDefaultAction(m_actNotOp);

    m_txtSearch = new QLineEdit(this);
    m_txtSearch->setFocusPolicy(Qt::StrongFocus);

    QToolButton *btnDisableMe = new QToolButton(this);
    btnDisableMe->setFocusPolicy(Qt::NoFocus);
    btnDisableMe->setDefaultAction(m_actDisableMe);

    QToolButton *btnRemoveMe = new QToolButton(this);
    btnRemoveMe->setFocusPolicy(Qt::NoFocus);
    btnRemoveMe->setDefaultAction(m_actRemoveMe);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->addWidget(m_cmbColumns);
    hLayout->addWidget(btnMatchCase);
    hLayout->addWidget(btnRegex);
    hLayout->addWidget(btnNotOp);
    hLayout->addWidget(m_txtSearch);
    hLayout->addWidget(btnDisableMe);
    hLayout->addWidget(btnRemoveMe);

    setLayout(hLayout);

    // setContentsMargins(0,0,0,0);

    createConnections();
}

SearchParamWidget::~SearchParamWidget()
{
}

void SearchParamWidget::createActions()
{
    m_actMatchCase = new QAction(tr("Match Case"), this);
    m_actMatchCase->setIcon(QIcon(":/images/case_sensitive_icon.png"));
    m_actMatchCase->setCheckable(true);

    m_actRegex = new QAction(tr("Regular Expression"), this);
    m_actRegex->setIcon(QIcon(":/images/regex_icon.png"));
    m_actRegex->setCheckable(true);

    m_actNotOp = new QAction(tr("Not (invert the match)"), this);
    m_actNotOp->setIcon(QIcon(":/images/not_icon.png"));
    m_actNotOp->setCheckable(true);

    m_actDisableMe = new QAction(tr("Disable Parameter"), this);
    m_actDisableMe->setIcon(QIcon(":/images/filter_off_icon.png"));
    m_actDisableMe->setCheckable(true);

    m_actRemoveMe = new QAction(tr("Remove Parameter"), this);
    m_actRemoveMe->setIcon(QIcon(":/images/delete_icon.png"));
}

void SearchParamWidget::createConnections()
{
    connect(m_txtSearch, &QLineEdit::returnPressed, this, &SearchParamWidget::searchRequested);
    connect(m_actRemoveMe, &QAction::triggered, this, [this]() { emit deleteRequested(this); });
}

void SearchParamWidget::setColumns(const tp::Columns  &columns)
{
    if ((columns.size() == 1) && columns.front().key.empty())
    {
        return;
    }

    for (const auto &column : columns)
    {
        m_cmbColumns->addItem(QString::fromStdString(column.name));
    }
}

bool SearchParamWidget::matchCase() const
{
    return m_actMatchCase->isChecked();
}

bool SearchParamWidget::isRegex() const
{
    return m_actRegex->isChecked();
}

bool SearchParamWidget::notOp() const
{
    return m_actNotOp->isChecked();
}

bool SearchParamWidget::getIsEnabled() const
{
    return !m_actDisableMe->isChecked();
}

std::string SearchParamWidget::expression() const
{
    return m_txtSearch->text().toStdString();
}

std::optional<tp::UInt> SearchParamWidget::column() const
{
    if (m_cmbColumns->currentIndex() > 0)
    {
        return m_cmbColumns->currentIndex() - 1;
    }

    return std::nullopt;
}
