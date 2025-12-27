// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "HeaderView.h"
#include "Style.h"
#include "QMouseEvent"
#include <QMenu>

// HeaderModel ----------------------------------------------------------------

priv::HeaderModel::HeaderModel(QObject *parent) : QAbstractTableModel(parent)
{
}

int priv::HeaderModel::rowCount(const QModelIndex & /*parent*/) const
{
    return 0;
}

int priv::HeaderModel::columnCount(const QModelIndex & /*parent*/) const
{
    return m_columns.size();
}

QVariant priv::HeaderModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QVariant priv::HeaderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:
            return QString::fromStdString(m_columns.at(section).get().name);
        case Qt::TextAlignmentRole:
            return Qt::AlignLeft;
        case Qt::FontRole:
            return Style::getFont();
        case Qt::BackgroundRole:
            return Style::getHeaderColor().bg;
        case Qt::ForegroundRole:
            return Style::getHeaderColor().fg;
        default:
            return QVariant();
    }
}

void priv::HeaderModel::setColumns(tp::Columns &columns)
{
    beginResetModel();
    m_columns.clear();
    m_columns.insert(m_columns.begin(), columns.begin(), columns.end());
    endResetModel();
}

// HeaderView -----------------------------------------------------------------

using namespace priv;

HeaderView::HeaderView(QWidget *parent) : QHeaderView(Qt::Horizontal, parent)
{
    setSectionResizeMode(QHeaderView::Interactive);
    setStretchLastSection(true);
    setSectionsMovable(true);
    // setSectionsClickable(true);

    m_headerModel = new HeaderModel(this);

    m_hide = new QAction(tr("Hide"), this);
    m_moveLeft = new QAction(tr("Move Left"), this);
    m_moveRight = new QAction(tr("Move Right"), this);
    m_expandColumn = new QAction(tr("Expand"), this);
    m_expandAllToContent = new QAction(Style::getIcon("expand_icon.png"), tr("Expand All"), this);
    m_expandAllToScreen = new QAction(Style::getIcon("fit_icon.png"), tr("Fit the Screen"), this);

    setModel(m_headerModel);

    connect(m_hide, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
    connect(m_moveLeft, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
    connect(m_moveRight, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
    connect(m_expandColumn, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
    connect(m_expandAllToContent, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
    connect(m_expandAllToScreen, &QAction::triggered, this, &HeaderView::handleContextMenuAction);

    enableBaseSignals();
}

HeaderView::~HeaderView()
{
}

void HeaderView::enableBaseSignals()
{
    connect(this, &QHeaderView::sectionCountChanged, this, &HeaderView::changedColumnsCount, Qt::UniqueConnection);
    connect(this, &QHeaderView::sectionMoved, this, &HeaderView::movedColumns, Qt::UniqueConnection);
    connect(this, &QHeaderView::sectionResized, this, &HeaderView::resizedColumn, Qt::UniqueConnection);
    connect(this, &QHeaderView::sectionDoubleClicked, this, &HeaderView::columnDoubleClicked, Qt::UniqueConnection);
    connect(this, &QHeaderView::geometriesChanged, this, &HeaderView::columnsChanged, Qt::UniqueConnection);
}

void HeaderView::disableBaseSignals()
{
    disconnect(this, &QHeaderView::sectionCountChanged, nullptr, nullptr);
    disconnect(this, &QHeaderView::sectionMoved, nullptr, nullptr);
    disconnect(this, &QHeaderView::sectionResized, nullptr, nullptr);
    disconnect(this, &QHeaderView::sectionDoubleClicked, nullptr, nullptr);
    disconnect(this, &QHeaderView::geometriesChanged, nullptr, nullptr);
}

void HeaderView::setColumns(tp::Columns &columns)
{
    m_headerModel->setColumns(columns);
}

void HeaderView::updateColumns()
{
    const auto &columns(m_headerModel->m_columns);
    for (size_t idx = 0; idx < columns.size(); ++idx)
    {
        auto &column = columns[idx].get();
        if (column.width == 0)
        {
            hideSection(column.idx);
        }
        else if (column.width > 0)
        {
            resizeSection(column.idx, column.width);
        }
    }
}

void HeaderView::changedColumnsCount(int, int)
{
    disableBaseSignals();

    auto &columns(m_headerModel->m_columns);
    for (size_t idx = 0; idx < columns.size(); ++idx)
    {
        auto &column = columns[idx].get();
        if (column.idx < 0)
        {
            column.idx = idx;
        }
        else if (column.idx != idx)
        {
            LOG_ERR("Column idx mismatch {} - {}", idx, column.idx);
            break;
        }

        if (column.pos < 0)
        {
            column.pos = visualIndex(idx);
        }
        else
        {
            const auto pos = visualIndex(column.idx);
            if (pos != -1)
            {
                moveSection(pos, column.pos);
            }
        }
    }

    if (!columns.empty())
        updateColumns();

    enableBaseSignals();
    emit columnsChanged();
}

void HeaderView::movedColumns(int, int, int)
{
    auto &columns(m_headerModel->m_columns);
    for (auto &column : columns)
    {
        column.get().pos = visualIndex(column.get().idx);
    }
    emit columnsChanged();
}

void HeaderView::resizedColumn(int idx, int oldSize, int size)
{
    auto &columns(m_headerModel->m_columns);
    if (idx < columns.size())
    {
        columns.at(idx).get().width = size;
    }
    emit columnsChanged();
}

void HeaderView::getVisibleColumns(tp::ColumnsRef &columnsRef, bool orderByPos)
{
    auto &columns(m_headerModel->m_columns);

    if (!orderByPos)
    {
        for (auto &column : columns)
        {
            if (column.get().width != 0)
            {
                columnsRef.emplace_back(column);
            }
        }
    }
    else
    {
        for (int pos = 0; pos < count(); ++pos)
        {
            const auto idx = logicalIndex(pos);
            if ((idx >= 0) && (idx < columns.size()))
            {
                auto &column = columns[idx];
                if (column.get().width != 0)
                {
                    columnsRef.emplace_back(column);
                }
            }
        }
    }
}

tp::ColumnsRef &HeaderView::getColumns()
{
    return m_headerModel->m_columns;
}

void HeaderView::mousePressEvent(QMouseEvent *e)
{
    if (e->buttons().testFlag(Qt::RightButton))
    {
        const auto idx = logicalIndexAt(e->pos());
        openContextMenu(e->globalPos(), idx);
        e->accept();
        return;
    }

    QHeaderView::mousePressEvent(e);
}

void HeaderView::columnDoubleClicked(int idx)
{
    emit expandToContent(idx);
}

void HeaderView::openContextMenu(QPoint pos, int idx)
{
    tp::ColumnsRef visibleColumns;
    // Gets the visible columns ordered by position
    getVisibleColumns(visibleColumns, true);

    // Gets the visual position of the column
    const auto getVisualPosFunc = [&visibleColumns](int idx) -> tp::SInt
    {
        tp::SInt visualPos(0);
        for (const auto &column : visibleColumns)
        {
            if (column.get().idx == idx)
            {
                return visualPos;
            }
            ++visualPos;
        }
        return -1L;
    };

    const auto visualPos = getVisualPosFunc(idx);
    if (visualPos < 0)
    {
        return;
    }

    QMenu menu(this);
    if (visibleColumns.size() > 1)
    {
        menu.addAction(m_hide);
    }

    if (hiddenSectionCount() > 0)
    {
        QMenu *showMenu = menu.addMenu(tr("Show"));
        for (int i = 0; i < count(); ++i)
        {
            if (isSectionHidden(i))
            {
                auto act = showMenu->addAction(m_headerModel->m_columns.at(i).get().name.c_str());
                act->setProperty("show", true);
                act->setProperty("column", i);
                connect(act, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
            }
        }
    }

    const bool canMoveLeft(visualPos > 0);
    const bool canMoveRight(visualPos < (visibleColumns.size() - 1));

    if (canMoveLeft || canMoveRight)
    {
        QMenu *moveMenu = menu.addMenu(tr("Move"));
        if (canMoveLeft)
        {
            int moveTo = visibleColumns.at(visualPos - 1).get().pos;
            m_moveLeft->setProperty("moveToPos", moveTo);
            moveMenu->addAction(m_moveLeft);
        }
        if (canMoveRight)
        {
            int moveTo = visibleColumns.at(visualPos + 1).get().pos;
            m_moveRight->setProperty("moveToPos", moveTo);
            moveMenu->addAction(m_moveRight);
        }
    }

    menu.addAction(m_expandColumn);

    menu.addSeparator();

    menu.addAction(m_expandAllToContent);
    menu.addAction(m_expandAllToScreen);

    m_contextColumn = idx;
    menu.exec(pos);
}

void HeaderView::handleContextMenuAction()
{
    const auto senderAct(sender());
    if (!m_contextColumn.has_value() || (senderAct == nullptr))
    {
        return;
    }

    const auto idx(m_contextColumn.value());
    const auto colPos(visualIndex(idx));

    if (senderAct == m_hide)
    {
        hideSection(m_contextColumn.value());
    }
    else if (senderAct == m_moveLeft)
    {
        int moveToPos = senderAct->property("moveToPos").toInt();
        moveSection(colPos, moveToPos);
    }
    else if (senderAct == m_moveRight)
    {
        int moveToPos = senderAct->property("moveToPos").toInt();
        moveSection(colPos, moveToPos);
    }
    else if (senderAct == m_expandColumn)
    {
        emit expandToContent(idx);
    }
    else if (senderAct == m_expandAllToContent)
    {
        emit expandAllToContent();
    }
    else if (senderAct == m_expandAllToScreen)
    {
        emit expandAllToScreen();
    }
    else if (senderAct->property("show") != QVariant())
    {
        int columnToShow = senderAct->property("column").toInt();
        showSection(columnToShow);
    }

    m_contextColumn = std::nullopt;
}
