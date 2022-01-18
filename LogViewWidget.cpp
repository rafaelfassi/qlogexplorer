#include "HeaderView.h"
#include "LogViewWidget.h"
#include "LongScrollBar.h"
#include "AbstractModel.h"
#include <QScrollBar>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QPushButton>
#include <QGuiApplication>
#include <QClipboard>
#include <QMenu>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <cmath>
#include <limits>
#include <iostream>

constexpr ssize_t g_scrollBarThickness(25);
constexpr ssize_t g_defaultMargin(10);
constexpr ssize_t g_startTextMargin(5);
constexpr auto g_fontName = "times";
constexpr int g_fontSize = 14;
static const QColor g_bgColor(Qt::white);
static const QColor g_txtColor(Qt::black);
static const QColor g_selectionBgColor("#4169e1");
static const QColor g_selectionTxtColor(Qt::white);
static const QColor g_headerTxtColor(Qt::white);
static const QColor g_headerBgColor(Qt::darkGray);
static const QColor g_markColor("#9400d3");

LogViewWidget::LogViewWidget(AbstractModel *model, QWidget *parent)
    : QWidget(parent),
      m_model(model),
      m_font(g_fontName, g_fontSize),
      m_fm(m_font)
{
    m_header = new HeaderView(this);
    m_header->setFont(&m_font);
    m_header->setTextColor(g_headerTxtColor);
    m_header->setBgColor(g_headerBgColor);
    m_header->setMaximumHeight(m_fm.height());
    m_header->setFixedHeight(m_header->maximumHeight());
    m_header->sizePolicy().setHorizontalPolicy(QSizePolicy::Expanding);
    m_header->sizePolicy().setVerticalPolicy(QSizePolicy::Fixed);

    m_vScrollBar = new LongScrollBar(Qt::Vertical, this);
    m_vScrollBar->setFixedWidth(g_scrollBarThickness);
    m_vScrollBar->sizePolicy().setVerticalPolicy(QSizePolicy::Expanding);
    m_vScrollBar->sizePolicy().setHorizontalPolicy(QSizePolicy::Fixed);

    m_hScrollBar = new LongScrollBar(Qt::Horizontal, this);
    m_hScrollBar->setPosPerStep(20);
    m_hScrollBar->setFixedHeight(g_scrollBarThickness);
    m_hScrollBar->sizePolicy().setHorizontalPolicy(QSizePolicy::Expanding);
    m_hScrollBar->sizePolicy().setVerticalPolicy(QSizePolicy::Fixed);

    m_btnExpandColumns = new QPushButton(this);
    m_btnExpandColumns->setFocusPolicy(Qt::NoFocus);
    m_btnExpandColumns->setIcon(QIcon(":/images/expand_icon.png"));
    m_btnExpandColumns->setToolTip("Expand All Columns");
    m_btnExpandColumns->setFlat(true);
    m_btnExpandColumns->setFixedHeight(m_header->maximumHeight());
    m_btnExpandColumns->setFixedWidth(g_scrollBarThickness);

    m_btnFitColumns = new QPushButton(this);
    m_btnFitColumns->setFocusPolicy(Qt::NoFocus);
    m_btnFitColumns->setIcon(QIcon(":/images/fit_icon.png"));
    m_btnFitColumns->setToolTip("Adjust Columns to Fit");
    m_btnFitColumns->setFlat(true);
    m_btnFitColumns->setFixedHeight(g_scrollBarThickness);
    m_btnFitColumns->setFixedWidth(g_scrollBarThickness);

    m_hScrollBarLayout = new QVBoxLayout();
    m_hScrollBarLayout->setMargin(0);
    m_hScrollBarLayout->addWidget(m_header, 0, Qt::AlignTop);
    m_hScrollBarLayout->addWidget(m_hScrollBar, 0, Qt::AlignBottom);

    m_vScrollBarLayout = new QVBoxLayout();
    m_vScrollBarLayout->setMargin(0);
    m_vScrollBarLayout->addWidget(m_btnExpandColumns, 0, Qt::AlignTop);
    m_vScrollBarLayout->addWidget(m_vScrollBar, 1, Qt::AlignRight);
    m_vScrollBarLayout->addWidget(m_btnFitColumns);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    hLayout->addLayout(m_hScrollBarLayout, 1);
    hLayout->addLayout(m_vScrollBarLayout);

    setLayout(hLayout);

    m_stabilizedUpdateTimer = new QTimer(this);
    m_stabilizedUpdateTimer->setInterval(200);

    m_actCopy = new QAction("Copy", this);
    m_actCopy->setShortcut(QKeySequence::Copy);
    m_actCopy->setShortcutContext(Qt::WidgetShortcut);

    m_actMark = new QAction("Mark", this);
    m_actMark->setShortcut(Qt::CTRL + Qt::Key_M);
    m_actMark->setShortcutContext(Qt::WidgetShortcut);

    m_actNextMark = new QAction("Next Mark", this);
    m_actNextMark->setShortcut(Qt::CTRL + Qt::Key_Down);
    m_actNextMark->setShortcutContext(Qt::WidgetShortcut);

    m_actPrevMark = new QAction("Previous Mark", this);
    m_actPrevMark->setShortcut(Qt::CTRL + Qt::Key_Up);
    m_actPrevMark->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_actCopy);
    addAction(m_actMark);
    addAction(m_actNextMark);
    addAction(m_actPrevMark);

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    connect(m_model, &AbstractModel::modelConfigured, this, &LogViewWidget::configureColumns, Qt::QueuedConnection);
    connect(m_model, &AbstractModel::countChanged, this, &LogViewWidget::modelCountChanged, Qt::QueuedConnection);
    connect(m_vScrollBar, &LongScrollBar::posChanged, this, &LogViewWidget::vScrollBarPosChanged);
    connect(m_hScrollBar, &LongScrollBar::posChanged, this, &LogViewWidget::hScrollBarPosChanged);
    connect(m_stabilizedUpdateTimer, &QTimer::timeout, this, &LogViewWidget::stabilizedUpdate);
    connect(m_header, &QHeaderView::geometriesChanged, this, &LogViewWidget::headerChanged);
    connect(m_header, &QHeaderView::sectionResized, this, [this](int, int, int) { this->headerChanged(); });
    connect(m_header, &QHeaderView::sectionMoved, this, [this](int, int, int) { this->headerChanged(); });
    connect(m_header, &HeaderView::expandToContent, this, &LogViewWidget::expandColumnToContent);
    connect(m_header, &HeaderView::expandAllToContent, this, [this]() { this->adjustColumns(ColumnsSize::Content); });
    connect(m_header, &HeaderView::expandAllToScreen, this, [this]() { this->adjustColumns(ColumnsSize::Screen); });
    connect(m_btnExpandColumns, &QPushButton::clicked, this, [this]() { this->adjustColumns(ColumnsSize::Content); });
    connect(m_btnFitColumns, &QPushButton::clicked, this, [this]() { this->adjustColumns(ColumnsSize::Screen); });
    connect(m_actCopy, &QAction::triggered, this, &LogViewWidget::copySelected);
    connect(m_actMark, &QAction::triggered, this, &LogViewWidget::markSelected);
    connect(m_actNextMark, &QAction::triggered, this, &LogViewWidget::goToNextMark);
    connect(m_actPrevMark, &QAction::triggered, this, &LogViewWidget::goToPrevMark);
}

LogViewWidget::~LogViewWidget()
{
}

AbstractModel *LogViewWidget::getModel()
{
    return m_model;
}

void LogViewWidget::headerChanged()
{
    updateView();
    m_stabilizedUpdateTimer->start();
}

void LogViewWidget::stabilizedUpdate()
{
    m_stabilizedUpdateTimer->stop();

    const ssize_t newMaxHScrollBar = std::max<ssize_t>(getMaxRowWidth() - m_textAreaRect.width(), 0L);
    if (newMaxHScrollBar != m_hScrollBar->getMax())
    {
        if (!m_vScrollBar->isKnobGrabbed() && !m_hScrollBar->isKnobGrabbed())
        {
            configureColumns();
            m_hScrollBar->setMax(newMaxHScrollBar);
        }
        else
        {
            m_stabilizedUpdateTimer->start();
        }
    }
}

void LogViewWidget::vScrollBarPosChanged()
{
    update();
}

void LogViewWidget::hScrollBarPosChanged()
{
    m_header->setOffset(m_hScrollBar->getPos());
    update();
}

void LogViewWidget::modelCountChanged()
{
    updateDisplaySize();
    update();
}

void LogViewWidget::goToRow(ssize_t row)
{
    if ((m_model != nullptr) && (row >= 0) && (row < m_model->rowCount()))
    {
        if ((row < m_vScrollBar->getPos()) || ((row - m_vScrollBar->getPos()) >= m_itemsPerPage))
        {
            m_vScrollBar->setPos(row - (m_itemsPerPage / 2));
        }

        m_currentRow = row;

        if (m_startSelect.has_value() || m_currentSelec.has_value())
        {
            m_startSelect = std::nullopt;
            m_currentSelec = std::nullopt;
        }

        update();
    }
}

void LogViewWidget::updateDisplaySize()
{
    if (m_model)
    {
        ssize_t rowCount(m_model->rowCount());
        m_rowHeight = m_fm.height();
        m_itemsPerPage = m_textAreaRect.height() / m_rowHeight;

        m_vScrollBar->setMax(rowCount - m_itemsPerPage);

        const std::string lastLineNumberStr(std::to_string(m_model->getRowNum(rowCount - 1L) + 1L));
        m_textAreaRect.setLeft(getTextWidth(lastLineNumberStr) + g_startTextMargin + g_defaultMargin);
        m_hScrollBarLayout->setContentsMargins(m_textAreaRect.left(), 0, 0, 0);
    }
    else
    {
        m_vScrollBar->setMax(-1);
        m_hScrollBar->setMax(-1);
    }
}

void LogViewWidget::updateView()
{
    updateDisplaySize();
    update();
}

void LogViewWidget::resizeEvent(QResizeEvent *event)
{
    m_textAreaRect.setHeight(height() - g_scrollBarThickness - m_textAreaRect.top());
    m_textAreaRect.setWidth(width() - g_scrollBarThickness - m_textAreaRect.left());
    updateDisplaySize();
    QWidget::resizeEvent(event);
    m_stabilizedUpdateTimer->start();
}

void LogViewWidget::paintEvent(QPaintEvent *event)
{
    const QRect invalidRect = event->rect();
    if (invalidRect.isEmpty())
        return;

    const auto &dataRect(m_textAreaRect);
    const auto &marks(m_marks);

    QPainter painter(this);
    painter.setFont(m_font);
    painter.eraseRect(rect());
    painter.setClipRect(dataRect);
    painter.fillRect(dataRect, g_bgColor);

    forEachVisualRowInPage(
        [&painter, &dataRect, &marks](VisualRowData &vrData)
        {
            // Draw Line Number
            {
                QRect numberAreaRect(vrData.numberRect);
                numberAreaRect.setLeft(0);
                painter.setClipping(false);
                painter.setPen(g_headerTxtColor);
                painter.fillRect(numberAreaRect, g_headerBgColor);
                if (marks.find(vrData.row) == marks.end())
                    painter.fillRect(numberAreaRect, g_headerBgColor);
                else
                    painter.fillRect(numberAreaRect, g_markColor);
                painter.drawText(
                    vrData.numberRect,
                    Qt::AlignTop | Qt::AlignLeft,
                    std::to_string(vrData.number + 1).c_str());
            }

            painter.setClipping(true);
            painter.setPen(g_txtColor);

            // Hightlight selected line
            if (vrData.selected)
            {
                painter.fillRect(vrData.rect, g_selectionBgColor);
                painter.setPen(g_selectionTxtColor);
            }

            // Draw Row Data
            for (const auto &colData : vrData.columns)
            {
                painter.drawText(colData.rect, Qt::AlignTop | Qt::AlignLeft, colData.text);

                if (colData.selection.has_value())
                {
                    const auto &[selRect, selText] = colData.selection.value();
                    painter.setClipRect(selRect.intersected(dataRect));
                    painter.fillRect(selRect, g_selectionBgColor);
                    painter.setPen(g_selectionTxtColor);
                    painter.drawText(colData.rect, Qt::AlignTop | Qt::AlignLeft, colData.text);
                    painter.setPen(g_txtColor);
                    painter.setClipRect(dataRect);
                }
            }
            return true;
        });

    m_stabilizedUpdateTimer->start();
}

void LogViewWidget::mousePressEvent(QMouseEvent *event)
{
    const ssize_t row = getRowByScreenPos(event->pos().y());
    const int xPos = event->pos().x();

    if (event->button() == Qt::LeftButton)
    {
        if (event->modifiers() == Qt::ShiftModifier && m_startSelect.has_value())
        {
            m_currentSelec = std::make_pair(row, xPos + m_hScrollBar->getPos());
        }
        else
        {
            m_startSelect = std::make_pair(row, xPos + m_hScrollBar->getPos());
            m_currentSelec = std::nullopt;
        }
        update();
    }
    else
    {
        if (!m_currentSelec.has_value())
        {
            m_currentRow = row;
        }

        m_actCopy->setEnabled(canCopy());
        m_actPrevMark->setEnabled(hasPrevMark());
        m_actNextMark->setEnabled(hasNextMark());

        QMenu menu(this);
        menu.addAction(m_actCopy);
        menu.addSeparator();
        menu.addAction(m_actMark);
        menu.addAction(m_actPrevMark);
        menu.addAction(m_actNextMark);
        menu.exec(event->globalPos());

        // Let the action enabled for the shortcuts
        m_actCopy->setEnabled(true);
        m_actPrevMark->setEnabled(true);
        m_actNextMark->setEnabled(true);
    }
}

void LogViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_currentSelec.has_value() && (event->button() == Qt::LeftButton))
    {
        const ssize_t row = getRowByScreenPos(event->pos().y());
        if (row >= 0 && (row < m_model->rowCount()))
        {
            m_currentRow = row;
            update();
            emit rowSelected(m_model->getRowNum(row));
        }
    }
}

void LogViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_startSelect.has_value())
    {
        const ssize_t row = getRowByScreenPos(event->pos().y());
        const ssize_t xPos = event->pos().x();

        if (xPos > m_textAreaRect.right())
        {
            const ssize_t offset = m_textAreaRect.right() - xPos;
            m_hScrollBar->setPos(m_hScrollBar->getPos() - offset);
        }
        else if (xPos < m_textAreaRect.left())
        {
            const ssize_t offset = xPos - m_textAreaRect.left();
            m_hScrollBar->setPos(m_hScrollBar->getPos() + offset);
        }
        else if (const auto lastRow = getLastPageRow(); row > lastRow)
        {
            const ssize_t offset = row - lastRow;
            m_vScrollBar->setPos(m_vScrollBar->getPos() + offset);
        }
        else if (const auto firstRow = getFirstPageRow(); row < firstRow)
        {
            const ssize_t offset = firstRow - row;
            m_vScrollBar->setPos(m_vScrollBar->getPos() - offset);
        }

        m_currentSelec = std::make_pair(row, xPos + m_hScrollBar->getPos());
        update();
    }
    else if (m_currentSelec.has_value())
    {
        m_currentSelec = std::nullopt;
    }
}

void LogViewWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    const int yPos(event->pos().y());
    const int xPos(event->pos().x());
    const ssize_t row = getRowByScreenPos(yPos);

    VisualRowData vrData;
    getVisualRowData(row, m_vScrollBar->getPos(), m_hScrollBar->getPos(), vrData);

    const auto isSeparator = [](const QChar &c)
    { return !c.isLetterOrNumber() && (c.category() != QChar::Punctuation_Connector); };

    for (const auto &col : vrData.columns)
    {
        if (!col.rect.contains(event->pos()))
            continue;

        const int textXPos = xPos - col.rect.left();
        const int chPos = getStrStartPos(col.text, textXPos);
        const auto wS = std::find_if(col.text.rbegin() + col.text.size() - chPos, col.text.rend(), isSeparator);
        const auto wE = std::find_if(col.text.begin() + chPos, col.text.end(), isSeparator);
        const auto sPos = std::distance(col.text.begin(), wS.base());
        const auto ePos = std::distance(col.text.begin(), wE);

        QString selText = col.text.mid(sPos, ePos - sPos);
        if (selText.isEmpty())
            break;

        const int selSize = m_fm.horizontalAdvance(selText);
        const int prevTextSize = m_fm.horizontalAdvance(col.text.mid(0, sPos));
        const int startSel = m_hScrollBar->getPos() + col.rect.left() + prevTextSize;
        const int lMargin = (m_fm.horizontalAdvance(selText.front()) / 2);
        const int rMargin = (m_fm.horizontalAdvance(selText.back()) / 3);

        m_startSelect = std::make_pair(row, startSel + lMargin);
        m_currentSelec = std::make_pair(row, startSel - rMargin + selSize);
        update();
        break;
    }
}

void LogViewWidget::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() == Qt::ShiftModifier)
    {
        m_hScrollBar->wheelEvent(event);
    }
    else
    {
        m_vScrollBar->wheelEvent(event);
    }
}

void LogViewWidget::getVisualRowData(ssize_t row, ssize_t rowOffset, ssize_t hOffset, VisualRowData &vrData)
{
    std::vector<std::string> rowData;
    const ssize_t relativeRow = row - rowOffset;
    const ssize_t yOffset = m_textAreaRect.top() + (m_rowHeight * relativeRow);

    vrData.row = row;
    vrData.number = m_model->getRow(row, rowData);

    QRect rect(m_textAreaRect.left(), yOffset, m_textAreaRect.width(), m_rowHeight);
    vrData.rect = rect;
    rect.translate(-hOffset, 0);

    vrData.numberRect = QRect(g_startTextMargin, yOffset, m_textAreaRect.left() - g_startTextMargin, m_rowHeight);

    std::optional<QRect> selectText;
    if (m_startSelect.has_value() && m_currentSelec.has_value())
    {
        const auto &[sRow, sPos] = m_startSelect.value();
        const auto &[eRow, ePos] = m_currentSelec.value();
        const auto fRow = std::min(sRow, eRow);
        const auto lRow = std::max(sRow, eRow);
        if (row >= fRow && row <= lRow)
        {
            if (sRow != eRow)
            {
                vrData.selected = true;
            }
            else if (sPos != ePos)
            {
                QRect selRect(rect);
                selRect.setLeft(std::min(sPos, ePos));
                selRect.setRight(std::max(sPos, ePos));
                selRect.translate(-hOffset, 0);
                selectText = selRect;
            }
        }
    }
    else if (m_currentRow.has_value() && m_currentRow.value() == row)
    {
        vrData.selected = true;
    }

    if (m_header->isVisible())
    {
        for (size_t vIdx = 0; vIdx < m_header->count(); ++vIdx)
        {
            VisualColData vcData;
            const ssize_t idx = m_header->logicalIndex(vIdx);
            const ssize_t colWidth = m_header->sectionSize(idx);
            rect.setWidth(colWidth);
            rect.setLeft(rect.left() + g_startTextMargin);
            if (idx < rowData.size())
            {
                vcData.text = getElidedText(rowData[idx], colWidth - g_defaultMargin, true);
                if (selectText.has_value() && rect.contains(selectText.value()))
                {
                    QRect selRect;
                    const auto &selText = getSelectedText(vcData.text, rect, selectText.value(), selRect);
                    if (!selText.isEmpty() && selRect.isValid())
                    {
                        vcData.selection = std::make_pair(selRect, selText);
                    }
                }
            }
            vcData.rect = rect;
            vrData.columns.emplace_back(std::move(vcData));
            rect.moveLeft(rect.left() + rect.width());
        }
    }
    else
    {
        for (const auto &colText : rowData)
        {
            VisualColData vcData;
            const ssize_t colWidth = getTextWidth(colText) + g_defaultMargin;
            rect.setWidth(colWidth);
            rect.setLeft(rect.left() + g_startTextMargin);
            vcData.text = QString::fromStdString(colText);
            if (selectText.has_value() && rect.contains(selectText.value()))
            {
                QRect selRect;
                const auto &selText = getSelectedText(vcData.text, rect, selectText.value(), selRect);
                if (!selText.isEmpty() && selRect.isValid())
                {
                    vcData.selection = std::make_pair(selRect, selText);
                }
            }
            vcData.rect = rect;
            vrData.columns.emplace_back(std::move(vcData));
            rect.moveLeft(rect.left() + rect.width());
        }
    }
}

void LogViewWidget::forEachVisualRowInPage(const std::function<bool(VisualRowData &)> &callback)
{
    const ssize_t rowsToRender(std::min<ssize_t>(m_itemsPerPage, m_model->rowCount()));
    for (ssize_t i = 0; i < rowsToRender; ++i)
    {
        VisualRowData vrData;
        const ssize_t row = i + m_vScrollBar->getPos();
        getVisualRowData(row, m_vScrollBar->getPos(), m_hScrollBar->getPos(), vrData);
        if (!callback(vrData))
        {
            break;
        }
    }
}

ssize_t LogViewWidget::getMaxRowWidth()
{
    ssize_t maxRowWidth(0);
    const auto offset(m_hScrollBar->getPos());
    forEachVisualRowInPage(
        [&maxRowWidth, offset](VisualRowData &vrData)
        {
            for (const auto &col : vrData.columns)
            {
                maxRowWidth = std::max<ssize_t>(maxRowWidth, col.rect.right() - vrData.numberRect.right() + offset);
            }
            return true;
        });
    return maxRowWidth;
}

ssize_t LogViewWidget::getFirstPageRow() const
{
    return m_vScrollBar->getPos();
}

ssize_t LogViewWidget::getLastPageRow() const
{
    const ssize_t itemsInPage(std::min<ssize_t>(m_itemsPerPage, m_model->rowCount()));
    return m_vScrollBar->getPos() + itemsInPage - 1;
}

ssize_t LogViewWidget::getRowByScreenPos(int yPos) const
{
    return m_vScrollBar->getPos() + ((yPos - m_textAreaRect.top()) / m_rowHeight);
}

ssize_t LogViewWidget::getTextWidth(const std::string &text, bool simplified)
{
    QString qStr(QString::fromStdString(text));
    return m_fm.horizontalAdvance(simplified ? qStr.simplified() : qStr);
}

QString LogViewWidget::getElidedText(const std::string &text, ssize_t width, bool simplified)
{
    QString qStr(QString::fromStdString(text));
    return m_fm.elidedText(simplified ? qStr.simplified() : qStr, Qt::ElideRight, width);
}

void LogViewWidget::configureColumns()
{
    if ((m_header->count() == 0) && !m_model->getColumns().empty())
    {
        m_header->setColumns(m_model->getColumns());
        if (m_model->getColumns().front().empty())
        {
            m_header->setVisible(false);
            m_btnExpandColumns->setVisible(false);
            m_btnFitColumns->setEnabled(false);
            m_btnFitColumns->setIcon(QIcon());
            m_btnFitColumns->setToolTip("");
        }
        else
        {
            m_textAreaRect.setTop(m_header->height());
            adjustColumns(ColumnsSize::Headers);
        }
        updateView();
    }
}

void LogViewWidget::getColumnsSizeToHeader(std::map<ssize_t, ssize_t> &columnSizesMap)
{
    ssize_t remainingWidth(m_textAreaRect.width());
    ssize_t remainingColumns(m_header->count() - m_header->hiddenSectionCount());
    ssize_t maxWidthPerCol = remainingWidth / std::max(remainingColumns, 1L);

    for (size_t idx = 0; idx < m_header->count(); ++idx)
    {
        if (!m_header->isSectionHidden(idx))
        {
            const std::string &headerText = m_header->getColumns().at(idx);
            const ssize_t headerTextWidth = getTextWidth(headerText) + g_startTextMargin + g_defaultMargin;
            const auto colSize = std::min(maxWidthPerCol, headerTextWidth);
            columnSizesMap[idx] = colSize;
            remainingWidth -= colSize;
            --remainingColumns;
            maxWidthPerCol = remainingWidth / std::max(remainingColumns, 1L);
        }
    }
}

void LogViewWidget::getColumnsSizeToContent(std::map<ssize_t, ssize_t> &columnSizesMap)
{
    getColumnsSizeToHeader(columnSizesMap);

    ssize_t rowsInPage(std::min<ssize_t>(m_itemsPerPage, m_model->rowCount()));
    const ssize_t elideWith(getTextWidth("..."));
    std::vector<std::string> rowData;

    for (ssize_t i = 0; i < rowsInPage; ++i)
    {
        ssize_t row = i + m_vScrollBar->getPos();
        rowData.clear();
        m_model->getRow(row, rowData);

        for (auto &[idx, columnWidth] : columnSizesMap)
        {
            const ssize_t textWidth =
                getTextWidth(rowData[idx], true) + g_startTextMargin + elideWith + g_defaultMargin;
            columnWidth = std::max(columnWidth, textWidth);
        }
    }
}

void LogViewWidget::getColumnsSizeToScreen(std::map<ssize_t, ssize_t> &columnSizesMap)
{
    getColumnsSizeToContent(columnSizesMap);

    std::map<ssize_t, ssize_t> columnsMap;
    ssize_t remainingWidth(m_textAreaRect.width());
    ssize_t remainingColumns(columnSizesMap.size());
    ssize_t maxWidthPerCol = remainingWidth / std::max(remainingColumns, 1L);
    std::pair<ssize_t, ssize_t> biggerColumn{0, 0};

    for (auto &[idx, columnWidth] : columnSizesMap)
    {
        if (columnWidth > biggerColumn.second)
        {
            biggerColumn.first = idx;
            biggerColumn.second = columnWidth;
        }

        columnWidth = std::min(maxWidthPerCol, columnWidth);
        remainingWidth -= columnWidth;
        --remainingColumns;
        maxWidthPerCol = remainingWidth / std::max(remainingColumns, 1L);
    }

    if (!columnSizesMap.empty() && remainingWidth > 0 && biggerColumn.second > 0)
    {
        columnSizesMap[biggerColumn.first] += remainingWidth;
    }
}

void LogViewWidget::adjustColumns(ColumnsSize size)
{
    std::map<ssize_t, ssize_t> columnSizesMap;

    switch (size)
    {
        case ColumnsSize::Headers:
            getColumnsSizeToHeader(columnSizesMap);
            break;
        case ColumnsSize::Content:
            getColumnsSizeToContent(columnSizesMap);
            break;
        case ColumnsSize::Screen:
            getColumnsSizeToScreen(columnSizesMap);
            break;
        default:
            break;
    }

    for (const auto &[idx, columnWidth] : columnSizesMap)
    {
        m_header->resizeSection(idx, columnWidth);
    }

    m_stabilizedUpdateTimer->start();
}

void LogViewWidget::expandColumnToContent(ssize_t columnIdx)
{
    std::map<ssize_t, ssize_t> columnSizesMap;
    getColumnsSizeToContent(columnSizesMap);
    const auto it = columnSizesMap.find(columnIdx);
    if (it != columnSizesMap.end())
    {
        m_header->resizeSection(it->first, it->second);
    }
}

int LogViewWidget::getStrStartPos(const QString &text, int left, int *newLeft)
{
    // Not the best implementation, but it allows variable-width fonts as well.
    int sX(0);
    int eX(m_fm.horizontalAdvance(text));
    int sPos(0);
    int x(sX);
    int p(sPos);
    QString tmpStr(text);

    while (x < left && !tmpStr.isEmpty())
    {
        sX = x;
        sPos = p;
        tmpStr = text.mid(++p);
        x = eX - m_fm.horizontalAdvance(tmpStr);
    }

    if (newLeft)
        *newLeft = sX;
    return sPos;
}

int LogViewWidget::getStrEndPos(const QString &text, int right, int *newRight)
{
    int eX(m_fm.horizontalAdvance(text));
    int ePos(text.size());
    int x(eX);
    int p(ePos);
    QString tmpStr(text);

    x = eX;
    p = ePos;
    tmpStr = text;
    while (x > right && !tmpStr.isEmpty())
    {
        eX = x;
        ePos = p;
        tmpStr = tmpStr.mid(0, --p);
        x = m_fm.horizontalAdvance(tmpStr);
    }

    if (newRight)
        *newRight = eX;
    return ePos;
}

QString LogViewWidget::getSelectedText(
    const QString &text,
    const QRect &textRect,
    const QRect &selRect,
    QRect &resultRect)
{
    if (text.isEmpty() || selRect.isNull() || !selRect.isValid())
    {
        resultRect = QRect();
        return QString();
    }

    const int selStart = selRect.left() - textRect.left();
    const int selEnd = selRect.right() - textRect.left();

    int sX;
    int eX;
    const int sPos = getStrStartPos(text, selStart, &sX);
    const int ePos = getStrEndPos(text, selEnd, &eX);

    if (eX <= sX || ePos <= sPos)
    {
        resultRect = QRect();
        return QString();
    }

    resultRect = textRect;
    resultRect.setLeft(textRect.left() + sX);
    resultRect.setRight(textRect.left() + eX);
    return text.mid(sPos, ePos - sPos);
}

bool LogViewWidget::canCopy() const
{
    return ((m_startSelect.has_value() && m_currentSelec.has_value()) || m_currentRow.has_value());
}

QString LogViewWidget::rowToText(ssize_t row)
{
    QString rowText;
    VisualRowData vrData;
    getVisualRowData(row, 0, 0, vrData);
    for (const auto &col : vrData.columns)
    {
        if (!rowText.isEmpty())
            rowText.append('\t');
        rowText.append(col.text);
    }
    return rowText;
}

QString LogViewWidget::getSelectedText(ssize_t row)
{
    QString selectedText;
    VisualRowData vrData;
    getVisualRowData(row, 0, 0, vrData);
    for (const auto &col : vrData.columns)
    {
        if (col.selection.has_value())
        {
            selectedText = col.selection->second;
            break;
        }
    }
    return selectedText;
}

void LogViewWidget::copySelected()
{
    QString value;

    if (m_startSelect.has_value() && m_currentSelec.has_value())
    {
        if (m_startSelect->first == m_currentSelec->first)
        {
            value = getSelectedText(m_startSelect->first);
        }
        else
        {
            const ssize_t firstRow = std::min(m_startSelect->first, m_currentSelec->first);
            const ssize_t lastRow = std::max(m_startSelect->first, m_currentSelec->first);
            for (ssize_t row = firstRow; row <= lastRow; ++row)
            {
                if (!value.isEmpty())
                    value.append('\n');
                value.append(rowToText(row));
            }
        }
    }
    else if (m_currentRow.has_value())
    {
        value = rowToText(m_currentRow.value());
    }

    if (!value.isEmpty())
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(value);
    }
}

const std::set<ssize_t> &LogViewWidget::getMarks()
{
    return m_marks;
}

bool LogViewWidget::hasMark(ssize_t row) const
{
    return (m_marks.find(row) != m_marks.end());
}

void LogViewWidget::clearMarks()
{
    m_marks.clear();
}

void LogViewWidget::markRow(ssize_t row)
{
    m_marks.insert(row);
}

void LogViewWidget::removeMark(ssize_t row)
{
    if (auto it = m_marks.find(row); it != m_marks.end())
    {
        m_marks.erase(it);
    }
}

void LogViewWidget::toggleMark(ssize_t row)
{
    if (auto it = m_marks.find(row); it != m_marks.end())
    {
        m_marks.erase(it);
        return;
    }
    m_marks.insert(row);
}

bool LogViewWidget::hasNextMark()
{
    if (!m_currentRow.has_value())
        return false;
    return (m_marks.upper_bound(m_currentRow.value()) != m_marks.end());
}

bool LogViewWidget::hasPrevMark()
{
    if (!m_currentRow.has_value())
        return false;
    return (m_marks.lower_bound(m_currentRow.value()) != m_marks.begin());
}

void LogViewWidget::goToNextMark()
{
    if (m_currentRow.has_value())
    {
        const auto it = m_marks.upper_bound(m_currentRow.value());
        if (it != m_marks.end())
        {
            goToRow(*it);
        }
    }
}

void LogViewWidget::goToPrevMark()
{
    if (m_currentRow.has_value())
    {
        auto it = m_marks.lower_bound(m_currentRow.value());
        if (it != m_marks.begin())
        {
            goToRow(*(--it));
        }
    }
}

void LogViewWidget::markSelected()
{
    if (m_startSelect.has_value() && m_currentSelec.has_value())
    {
        const ssize_t firstRow = std::min(m_startSelect->first, m_currentSelec->first);
        const ssize_t lastRow = std::max(m_startSelect->first, m_currentSelec->first);
        bool allMarked(true);
        for (ssize_t row = firstRow; row <= lastRow; ++row)
        {
            if (!hasMark(row))
            {
                allMarked = false;
                break;
            }
        }
        for (ssize_t row = firstRow; row <= lastRow; ++row)
        {
            if (allMarked)
                removeMark(row);
            else
                markRow(row);
        }
    }
    else if (m_currentRow.has_value())
    {
        toggleMark(m_currentRow.value());
    }

    update();
}