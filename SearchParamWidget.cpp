#include "SearchParamWidget.h"
#include "LogViewWidget.h"
#include "TextLogModel.h"
#include "JsonLogModel.h"
#include "ProxyModel.h"
#include "LongScrollBar.h"
#include <QTableView>
#include <QDebug>
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

    QToolButton *btnWholeText = new QToolButton(this);
    btnWholeText->setFocusPolicy(Qt::NoFocus);
    btnWholeText->setDefaultAction(m_actWholeText);

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
    hLayout->addWidget(btnWholeText);
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

    m_actWholeText = new QAction(tr("Match Whole Text"), this);
    m_actWholeText->setIcon(QIcon(":/images/whole_text_icon.png"));
    m_actWholeText->setCheckable(true);

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

void SearchParamWidget::setColumns(const std::vector<std::string> &columns)
{
    if ((columns.size() == 1) && columns.front().empty())
    {
        return;
    }

    for (const auto &column : columns)
    {
        m_cmbColumns->addItem(QString::fromStdString(column));
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

bool SearchParamWidget::matchWholeText() const
{
    return m_actWholeText->isChecked();
}

bool SearchParamWidget::getIsEnabled() const
{
    return !m_actDisableMe->isChecked();
}

std::string SearchParamWidget::expression() const
{
    return m_txtSearch->text().toStdString();
}

std::optional<std::size_t> SearchParamWidget::column() const
{
    if (m_cmbColumns->currentIndex() > 0)
    {
        return m_cmbColumns->currentIndex() - 1;
    }

    return std::nullopt;
}
