#include "HeaderView.h"
#include "QMouseEvent"
#include <QMenu>
#include <QDebug>

// HeaderModel ----------------------------------------------------------------

HeaderModel::HeaderModel(QObject *parent) : QAbstractTableModel(parent)
{
}

int HeaderModel::rowCount(const QModelIndex & /*parent*/) const
{
    return 0;
}

int HeaderModel::columnCount(const QModelIndex & /*parent*/) const
{
    return m_columns.size();
}

QVariant HeaderModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QVariant HeaderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:
            return QString::fromStdString(m_columns.at(section));
        case Qt::TextAlignmentRole:
            return Qt::AlignLeft;
        case Qt::FontRole:
            return (m_font != nullptr) ? *m_font : QFont();
        case Qt::BackgroundRole:
            return QBrush(QColor("blue"));
        case Qt::ForegroundRole:
            return QBrush(QColor("white"));
        default:
            return QVariant();
    }
}

void HeaderModel::addColumn(const std::string &colName)
{
    beginInsertColumns(QModelIndex(), m_columns.size(), m_columns.size());
    m_columns.push_back(colName);
    endInsertColumns();
}

void HeaderModel::setColumns(const std::vector<std::string> &columns)
{
    beginResetModel();
    m_columns.insert(m_columns.begin(), columns.begin(), columns.end());
    endResetModel();
}

// HeaderView -----------------------------------------------------------------

HeaderView::HeaderView(QWidget *parent) : QHeaderView(Qt::Horizontal, parent)
{
    setSectionResizeMode(QHeaderView::Interactive);
    setStretchLastSection(true);
    setSectionsMovable(true);
    // setSectionsClickable(true);

    m_headerModel = new HeaderModel(this);

    m_hide = new QAction("Hide", this);
    m_moveLeft = new QAction("Move Left", this);
    m_moveRight = new QAction("Move Right", this);
    m_expandColumn = new QAction("Expand", this);
    m_expandAllToContent = new QAction(QIcon(":/images/expand_icon.png"), "Expand All", this);
    m_expandAllToScreen = new QAction(QIcon(":/images/fit_icon.png"), "Fit the Screen", this);

    setModel(m_headerModel);

    connect(this, &QHeaderView::sectionDoubleClicked, this, &HeaderView::columnDoubleClicked);
    connect(m_hide, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
    connect(m_moveLeft, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
    connect(m_moveRight, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
    connect(m_expandColumn, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
    connect(m_expandAllToContent, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
    connect(m_expandAllToScreen, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
}

HeaderView::~HeaderView()
{
}

void HeaderView::setColumns(const std::vector<std::string> &columns)
{
    m_headerModel->setColumns(columns);
}

const std::vector<std::string> &HeaderView::getColumns()
{
    return m_headerModel->m_columns;
}

void HeaderView::setFont(const QFont *font)
{
    m_headerModel->m_font = font;
}

void HeaderView::getVisibleColumns(VisibleColumns &columns)
{
    for (int pos = 0; pos < count(); ++pos)
    {
        const auto idx = logicalIndex(pos);
        if ((idx >= 0) && !isSectionHidden(idx))
        {
            columns.push_back(std::make_pair<ssize_t, ssize_t>(idx, pos));
        }
    }
}

VisibleColPos HeaderView::getVisiblePos(ssize_t colIdx, const VisibleColumns &visibleColumns)
{
    ssize_t visiblePos(0);
    for (const auto &[cIdx, cPos] : visibleColumns)
    {
        if (cIdx == colIdx)
        {
            return std::make_pair(visiblePos, cPos);
        }
        ++visiblePos;
    }

    return {-1, -1};
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
    VisibleColumns visibleColumns;
    getVisibleColumns(visibleColumns);
    const auto &[visiblePos, colPos] = getVisiblePos(idx, visibleColumns);
    if (visiblePos < 0)
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
        QMenu *showMenu = menu.addMenu("Show");
        for (int i = 0; i < count(); ++i)
        {
            if (isSectionHidden(i))
            {
                auto act = showMenu->addAction(m_headerModel->m_columns.at(i).c_str());
                act->setProperty("show", true);
                act->setProperty("column", i);
                connect(act, &QAction::triggered, this, &HeaderView::handleContextMenuAction);
            }
        }
    }

    const bool canMoveLeft(visiblePos > 0);
    const bool canMoveRight(visiblePos < (visibleColumns.size() - 1));

    if (canMoveLeft || canMoveRight)
    {
        qDebug() << "------------------";
        qDebug() << "idx" << idx;
        qDebug() << "colPos" << colPos;
        qDebug() << "visiblePos" << visiblePos;

        QMenu *moveMenu = menu.addMenu("Move");
        if (canMoveLeft)
        {
            int moveTo = visibleColumns.at(visiblePos - 1).second;
            m_moveLeft->setProperty("moveToPos", moveTo);
            moveMenu->addAction(m_moveLeft);
        }
        if (canMoveRight)
        {
            int moveTo = visibleColumns.at(visiblePos + 1).second;
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
        qDebug() << "moveToPos" << moveToPos;
        moveSection(colPos, moveToPos);
    }
    else if (senderAct == m_moveRight)
    {
        int moveToPos = senderAct->property("moveToPos").toInt();
        qDebug() << "moveToPos" << moveToPos;
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