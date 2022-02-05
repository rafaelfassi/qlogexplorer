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

    QToolButton *btnRange = new QToolButton(this);
    btnRange->setFocusPolicy(Qt::NoFocus);
    btnRange->setDefaultAction(m_actRange);

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
    hLayout->addWidget(btnRange);
    hLayout->addWidget(btnNotOp);
    hLayout->addWidget(m_txtSearch);
    hLayout->addWidget(btnDisableMe);
    hLayout->addWidget(btnRemoveMe);

    setLayout(hLayout);

    // setContentsMargins(0,0,0,0);

    createConnections();

    updateOptions();
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

    m_actRange = new QAction(tr("Range match (from -> to)"), this);
    m_actRange->setIcon(QIcon(":/images/range_icon.png"));
    m_actRange->setCheckable(true);

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
    connect(m_actRange, &QAction::triggered, this, &SearchParamWidget::updateOptions);
    connect(m_cmbColumns, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() { updateOptions(); });
}

void SearchParamWidget::updateOptions()
{
    m_actRange->setEnabled(m_cmbColumns->currentIndex() > 0);
    if (!m_actRange->isEnabled())
        m_actRange->setChecked(false);

    bool rangeActive(m_actRange->isEnabled() && m_actRange->isChecked());
    m_actMatchCase->setEnabled(!rangeActive);
    m_actRegex->setEnabled(!rangeActive);
}

void SearchParamWidget::setColumns(const tp::Columns &columns)
{
    m_columns = columns;

    if ((columns.size() == 1) && columns.front().key.empty())
    {
        return;
    }

    for (const auto &column : m_columns)
    {
        m_cmbColumns->addItem(QString::fromStdString(column.name));
    }
}

tp::SearchType SearchParamWidget::getSearchType()
{
    if (m_actRange->isEnabled() && m_actRange->isChecked())
        return tp::SearchType::Range;
    else if (m_actRegex->isEnabled() && m_actRegex->isChecked())
        return tp::SearchType::Regex;
    else
        return tp::SearchType::SubString;
}

tp::SearchFlags SearchParamWidget::getSearchFlags()
{
    tp::SearchFlags flags;
    if (m_actMatchCase->isEnabled() && m_actMatchCase->isChecked())
        flags.set(tp::SearchFlag::MatchCase);
    if (m_actNotOp->isEnabled() && m_actNotOp->isChecked())
        flags.set(tp::SearchFlag::NotOperator);
    return flags;
}

bool SearchParamWidget::getIsEnabled() const
{
    return !m_actDisableMe->isChecked();
}

std::string SearchParamWidget::expression() const
{
    return m_txtSearch->text().toStdString();
}

std::optional<tp::Column> SearchParamWidget::column() const
{
    const auto colIdx = m_cmbColumns->currentIndex() - 1;
    if (colIdx >= 0 & colIdx < m_columns.size())
    {
        return m_columns.at(colIdx);
    }

    return std::nullopt;
}
