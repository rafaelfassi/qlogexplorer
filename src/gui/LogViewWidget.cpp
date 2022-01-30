#include "pch.h"
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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <cmath>
#include <limits>

constexpr tp::SInt g_scrollBarThickness(25);
constexpr tp::SInt g_defaultMargin(10);
constexpr tp::SInt g_startTextMargin(5);
constexpr auto g_fontName = "times";
constexpr tp::SInt g_fontSize = 14;
static const QColor g_bgColor(Qt::white);
static const QColor g_txtColor(Qt::black);
static const QColor g_selectionBgColor("#4169e1");
static const QColor g_selectionTxtColor(Qt::white);
static const QColor g_headerTxtColor(Qt::white);
static const QColor g_headerBgColor(Qt::darkGray);
static const QColor g_markBgColor("#9400d3");

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

    m_actGoUp = new QAction("Up", this);
    m_actGoUp->setShortcut(QKeySequence::MoveToPreviousLine);
    m_actGoUp->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actGoUp);

    m_actGoDown = new QAction("Down", this);
    m_actGoDown->setShortcut(QKeySequence::MoveToNextLine);
    m_actGoDown->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actGoDown);

    m_actGoPrevPage = new QAction("Previous Page", this);
    m_actGoPrevPage->setShortcut(QKeySequence::MoveToPreviousPage);
    m_actGoPrevPage->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actGoPrevPage);

    m_actGoNextPage = new QAction("Next Page", this);
    m_actGoNextPage->setShortcut(QKeySequence::MoveToNextPage);
    m_actGoNextPage->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actGoNextPage);

    m_actGoFirstRow = new QAction("First Row", this);
    m_actGoFirstRow->setShortcut(QKeySequence::MoveToStartOfDocument);
    m_actGoFirstRow->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actGoFirstRow);

    m_actGoLastRow = new QAction("Last Row", this);
    m_actGoLastRow->setShortcut(QKeySequence::MoveToEndOfDocument);
    m_actGoLastRow->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actGoLastRow);

    m_actGoLeft = new QAction("Left", this);
    m_actGoLeft->setShortcut(QKeySequence::MoveToPreviousChar);
    m_actGoLeft->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actGoLeft);

    m_actGoRight = new QAction("Right", this);
    m_actGoRight->setShortcut(QKeySequence::MoveToNextChar);
    m_actGoRight->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actGoRight);

    m_actGoFullLeft = new QAction("Start of Line", this);
    m_actGoFullLeft->setShortcut(QKeySequence::MoveToStartOfLine);
    m_actGoFullLeft->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actGoFullLeft);

    m_actGoFullRight = new QAction("End of Line", this);
    m_actGoFullRight->setShortcut(QKeySequence::MoveToEndOfLine);
    m_actGoFullRight->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actGoFullRight);

    m_actCopy = new QAction("Copy", this);
    m_actCopy->setShortcut(QKeySequence::Copy);
    m_actCopy->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actCopy);

    m_actMark = new QAction("Toggle Mark", this);
    m_actMark->setShortcut(Qt::CTRL + Qt::Key_M);
    m_actMark->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actMark);

    m_actPrevMark = new QAction("Previous Mark", this);
    m_actPrevMark->setShortcut(Qt::CTRL + Qt::Key_Up);
    m_actPrevMark->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actPrevMark);

    m_actNextMark = new QAction("Next Mark", this);
    m_actNextMark->setShortcut(Qt::CTRL + Qt::Key_Down);
    m_actNextMark->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actNextMark);

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    connect(m_model, &AbstractModel::modelConfigured, this, &LogViewWidget::configureColumns, Qt::QueuedConnection);
    connect(m_model, &AbstractModel::countChanged, this, &LogViewWidget::modelCountChanged, Qt::QueuedConnection);
    connect(m_vScrollBar, &LongScrollBar::posChanged, this, &LogViewWidget::vScrollBarPosChanged);
    connect(m_hScrollBar, &LongScrollBar::posChanged, this, &LogViewWidget::hScrollBarPosChanged);
    connect(m_stabilizedUpdateTimer, &QTimer::timeout, this, &LogViewWidget::stabilizedUpdate);
    connect(m_header, &HeaderView::columnsChanged, this, &LogViewWidget::headerChanged);
    connect(m_header, &HeaderView::expandToContent, this, &LogViewWidget::expandColumnToContent);
    connect(m_header, &HeaderView::expandAllToContent, this, [this]() { this->adjustColumns(ColumnsSize::Content); });
    connect(m_header, &HeaderView::expandAllToScreen, this, [this]() { this->adjustColumns(ColumnsSize::Screen); });
    connect(m_btnExpandColumns, &QPushButton::clicked, this, [this]() { this->adjustColumns(ColumnsSize::Content); });
    connect(m_btnFitColumns, &QPushButton::clicked, this, [this]() { this->adjustColumns(ColumnsSize::Screen); });
    connect(m_actCopy, &QAction::triggered, this, &LogViewWidget::copySelected);
    connect(m_actMark, &QAction::triggered, this, &LogViewWidget::markSelected);
    connect(m_actPrevMark, &QAction::triggered, this, &LogViewWidget::goToPrevMark);
    connect(m_actNextMark, &QAction::triggered, this, &LogViewWidget::goToNextMark);
    connect(m_actGoUp, &QAction::triggered, this, &LogViewWidget::goToPrevRow);
    connect(m_actGoDown, &QAction::triggered, this, &LogViewWidget::goToNextRow);
    connect(m_actGoPrevPage, &QAction::triggered, this, &LogViewWidget::goToPrevPage);
    connect(m_actGoNextPage, &QAction::triggered, this, &LogViewWidget::goToNextPage);
    connect(m_actGoFirstRow, &QAction::triggered, this, &LogViewWidget::gotToFirstRow);
    connect(m_actGoLastRow, &QAction::triggered, this, &LogViewWidget::gotToLastRow);
    connect(m_actGoLeft, &QAction::triggered, this, &LogViewWidget::goLeft);
    connect(m_actGoRight, &QAction::triggered, this, &LogViewWidget::goRight);
    connect(m_actGoFullLeft, &QAction::triggered, this, &LogViewWidget::goFullLeft);
    connect(m_actGoFullRight, &QAction::triggered, this, &LogViewWidget::goFullRight);
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

    const tp::SInt newMaxHScrollBar = std::max<tp::SInt>(getMaxRowWidth() - m_textAreaRect.width(), 0L);
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

void LogViewWidget::goToRow(tp::SInt row)
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
        tp::SInt rowCount(m_model->rowCount());
        m_rowHeight = m_fm.height();
        m_itemsPerPage = m_textAreaRect.height() / m_rowHeight;

        m_vScrollBar->setMax(rowCount - m_itemsPerPage);

        const std::string lastLineNumberStr(std::to_string(m_model->getRowNum(rowCount - 1L) + 1L));
        m_textAreaRect.setLeft(getTextWidth(lastLineNumberStr) + 2 * g_startTextMargin);
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

void LogViewWidget::resetColumns()
{
    tp::Columns emptyColumns;
    m_header->setColumns(emptyColumns);
}

void LogViewWidget::resizeEvent(QResizeEvent *event)
{
    m_textAreaRect.setHeight(height() - g_scrollBarThickness - m_textAreaRect.top());
    m_textAreaRect.setWidth(width() - g_scrollBarThickness - m_textAreaRect.left());
    updateDisplaySize();
    QWidget::resizeEvent(event);
    m_stabilizedUpdateTimer->start();
}

void LogViewWidget::mousePressEvent(QMouseEvent *event)
{
    const tp::SInt row = getRowByScreenPos(event->pos().y());
    const int xPos = event->pos().x();

    if (event->button() == Qt::LeftButton)
    {
        if (m_startSelect.has_value() && (event->modifiers() == Qt::ShiftModifier))
        {
            // Selects many rows by clicking with shift.
            m_currentSelec = std::make_pair(row, xPos + m_hScrollBar->getPos());
        }
        else
        {

            m_startSelect = std::make_pair(row, xPos + m_hScrollBar->getPos());
            m_currentSelec = std::nullopt;
            if (m_currentRow != row)
            {
                m_currentRow = row;
                emit rowSelected(m_model->getRowNum(row));
            }
        }
        update();
    }
    else
    {
        if (!m_currentSelec.has_value() && (m_currentRow != row))
        {
            m_currentRow = row;
            emit rowSelected(m_model->getRowNum(row));
        }

        // As an alternative implementation, it could not include the not available actions into the menu.
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

void LogViewWidget::mouseReleaseEvent(QMouseEvent *)
{
    // if (!m_currentSelec.has_value() && (event->button() == Qt::LeftButton))
    // {
    //     const tp::SInt row = getRowByScreenPos(event->pos().y());
    //     if (row >= 0 && (row < m_model->rowCount()))
    //     {
    //         m_currentRow = row;
    //         update();
    //         emit rowSelected(m_model->getRowNum(row));
    //     }
    // }
}

void LogViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_startSelect.has_value())
    {
        const tp::SInt row = getRowByScreenPos(event->pos().y());
        const tp::SInt xPos = event->pos().x();

        if (xPos > m_textAreaRect.right())
        {
            const tp::SInt offset = m_textAreaRect.right() - xPos;
            m_hScrollBar->setPos(m_hScrollBar->getPos() - offset);
        }
        else if (xPos < m_textAreaRect.left())
        {
            const tp::SInt offset = xPos - m_textAreaRect.left();
            m_hScrollBar->setPos(m_hScrollBar->getPos() + offset);
        }
        else if (const auto lastRow = getLastPageRow(); row > lastRow)
        {
            const tp::SInt offset = row - lastRow;
            m_vScrollBar->setPos(m_vScrollBar->getPos() + offset);
        }
        else if (const auto firstRow = getFirstPageRow(); row < firstRow)
        {
            const tp::SInt offset = firstRow - row;
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
    const tp::SInt row = getRowByScreenPos(yPos);

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

void LogViewWidget::paintEvent(QPaintEvent *event)
{
    const QRect invalidRect = event->rect();
    if (invalidRect.isEmpty())
        return;

    QPainter painter(this);
    painter.setFont(m_font);
    painter.eraseRect(rect());
    painter.setClipRect(m_textAreaRect);
    painter.fillRect(m_textAreaRect, g_bgColor);

    forEachVisualRowInPage(
        [&painter, this](VisualRowData &vrData)
        {
            // Draw Line Number
            {
                painter.setClipping(false);
                painter.setPen(g_headerTxtColor);
                const QColor &bgColor = hasMark(vrData.row) ? g_markBgColor : g_headerBgColor;
                painter.fillRect(vrData.numberAreaRect, bgColor);
                painter.drawText(
                    vrData.numberRect,
                    Qt::AlignTop | Qt::AlignRight,
                    std::to_string(vrData.number + 1).c_str());
            }

            QColor currentTextColor(g_txtColor);
            painter.setClipping(true);

            // Selected line
            if (vrData.selected)
            {
                painter.fillRect(vrData.rect, g_selectionBgColor);
                currentTextColor = g_selectionTxtColor;
            }
            else if (vrData.highlighter != nullptr)
            {
                painter.fillRect(vrData.rect, vrData.highlighter->getBgColor());
                currentTextColor = vrData.highlighter->getTextColor();
            }

            painter.setPen(currentTextColor);

            // Draw Row Data
            for (const auto &colData : vrData.columns)
            {
                // Draw column text
                painter.drawText(colData.rect, Qt::AlignTop | Qt::AlignLeft, colData.text);

                if (colData.selection.has_value())
                {
                    // Draw selected text
                    const auto &[selRect, selText] = colData.selection.value();
                    painter.setClipRect(selRect.intersected(m_textAreaRect));
                    painter.fillRect(selRect, g_selectionBgColor);
                    painter.setPen(g_selectionTxtColor);
                    painter.drawText(colData.rect, Qt::AlignTop | Qt::AlignLeft, colData.text);
                    painter.setPen(currentTextColor);
                    painter.setClipRect(m_textAreaRect);
                }
            }
            return true;
        });

    m_stabilizedUpdateTimer->start();
}

void LogViewWidget::getVisualRowData(tp::SInt row, tp::SInt rowOffset, tp::SInt hOffset, VisualRowData &vrData)
{
    tp::RowData rowData;
    const tp::SInt relativeRow = row - rowOffset;
    const tp::SInt yOffset = m_textAreaRect.top() + (m_rowHeight * relativeRow);

    vrData.row = row;
    vrData.number = m_model->getRow(row, rowData);

    QRect rect(m_textAreaRect.left(), yOffset, m_textAreaRect.width(), m_rowHeight);
    vrData.rect = rect;
    rect.translate(-hOffset, 0);

    vrData.numberAreaRect = QRect(0, yOffset, m_textAreaRect.left(), m_rowHeight);
    vrData.numberRect = QRect(g_startTextMargin, yOffset, m_textAreaRect.left() - g_startTextMargin * 2, m_rowHeight);

    for (const auto &highlighter : m_highlighters)
    {
        if (highlighter.matchInRow(rowData))
        {
            vrData.highlighter = &highlighter;
            break;
        }
    }

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
        for (int vIdx = 0; vIdx < m_header->count(); ++vIdx)
        {
            VisualColData vcData;
            const tp::SInt idx = m_header->logicalIndex(vIdx);
            const tp::SInt colWidth = m_header->sectionSize(idx);
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
            const tp::SInt colWidth = getTextWidth(colText) + g_defaultMargin;
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
    const tp::SInt rowsToRender(std::min<tp::SInt>(m_itemsPerPage, m_model->rowCount()));
    for (tp::SInt i = 0; i < rowsToRender; ++i)
    {
        VisualRowData vrData;
        const tp::SInt row = i + m_vScrollBar->getPos();
        getVisualRowData(row, m_vScrollBar->getPos(), m_hScrollBar->getPos(), vrData);
        if (!callback(vrData))
        {
            break;
        }
    }
}

tp::SInt LogViewWidget::getMaxRowWidth()
{
    tp::SInt maxRowWidth(0);
    const auto offset(m_hScrollBar->getPos());
    forEachVisualRowInPage(
        [&maxRowWidth, offset](VisualRowData &vrData)
        {
            for (const auto &col : vrData.columns)
            {
                maxRowWidth = std::max<tp::SInt>(maxRowWidth, col.rect.right() - vrData.numberAreaRect.right() + offset);
            }
            return true;
        });
    return maxRowWidth;
}

tp::SInt LogViewWidget::getFirstPageRow() const
{
    return m_vScrollBar->getPos();
}

tp::SInt LogViewWidget::getLastPageRow() const
{
    const tp::SInt itemsInPage(std::min<tp::SInt>(m_itemsPerPage, m_model->rowCount()));
    return m_vScrollBar->getPos() + itemsInPage - 1;
}

tp::SInt LogViewWidget::getRowByScreenPos(int yPos) const
{
    return m_vScrollBar->getPos() + ((yPos - m_textAreaRect.top()) / m_rowHeight);
}

tp::SInt LogViewWidget::getTextWidth(const std::string &text, bool simplified)
{
    QString qStr(QString::fromStdString(text));
    return m_fm.horizontalAdvance(simplified ? qStr.simplified() : qStr);
}

QString LogViewWidget::getElidedText(const std::string &text, tp::SInt width, bool simplified)
{
    QString qStr(QString::fromStdString(text));
    return m_fm.elidedText(simplified ? qStr.simplified() : qStr, Qt::ElideRight, width);
}

void LogViewWidget::configure(Conf *conf)
{
    m_highlighters.clear();
    for (const auto & param : conf->getHighlighterParams())
    {
        m_highlighters.push_back(Highlighter(param));
    }
}

void LogViewWidget::configureColumns()
{
    if ((m_header->count() == 0) && !m_model->getColumns().empty())
    {
        m_header->setColumns(m_model->getColumns());
        if (m_model->getColumns().front().key.empty())
        {
            m_header->setVisible(false);
            m_btnExpandColumns->setVisible(false);
            m_btnFitColumns->setEnabled(false);
            m_textAreaRect.setTop(0);
            // m_btnFitColumns->setIcon(QIcon());
            // m_btnFitColumns->setToolTip("");
        }
        else
        {
            m_header->setVisible(true);
            m_btnExpandColumns->setVisible(true);
            m_btnFitColumns->setEnabled(true);
            m_textAreaRect.setTop(m_header->height());
            adjustColumns(ColumnsSize::Headers);
        }
        updateView();
    }
}

void LogViewWidget::getColumnsSizeToHeader(tp::ColumnsRef &columnsRef, bool discardConfig)
{
    for (auto &headerColumn : columnsRef)
    {
        auto &column(headerColumn.get());
        if (discardConfig || (column.width < 0))
        {
            column.width = getTextWidth(column.name) + g_startTextMargin + g_defaultMargin;
        }
    }
}

void LogViewWidget::getColumnsSizeToContent(tp::ColumnsRef &columnsRef)
{
    getColumnsSizeToHeader(columnsRef, true);

    tp::SInt rowsInPage(std::min<tp::SInt>(m_itemsPerPage, m_model->rowCount()));
    const tp::SInt elideWith(getTextWidth("..."));
    tp::RowData rowData;

    for (tp::SInt i = 0; i < rowsInPage; ++i)
    {
        tp::SInt row = i + m_vScrollBar->getPos();
        rowData.clear();
        m_model->getRow(row, rowData);

        for (auto &headerColumn : columnsRef)
        {
            auto &column(headerColumn.get());
            const tp::SInt textWidth =
                getTextWidth(rowData[column.idx], true) + g_startTextMargin + elideWith + g_defaultMargin;
            column.width = std::max<tp::SInt>(column.width, textWidth);
        }
    }
}

void LogViewWidget::getColumnsSizeToScreen(tp::ColumnsRef &columnsRef)
{
    getColumnsSizeToContent(columnsRef);

    std::map<tp::SInt, tp::SInt> columnsMap;
    tp::SInt remainingWidth(m_textAreaRect.width());
    tp::SInt remainingColumns(columnsRef.size());
    tp::SInt maxWidthPerCol = remainingWidth / std::max(remainingColumns, 1L);
    std::pair<tp::SInt, tp::SInt> biggerColumn{0, 0};

    tp::SInt idx(0);
    for (auto &headerColumn : columnsRef)
    {
        auto &column(headerColumn.get());
        if (column.width > biggerColumn.second)
        {
            biggerColumn.first = idx;
            biggerColumn.second = column.width;
        }

        column.width = std::min<tp::SInt>(maxWidthPerCol, column.width);
        remainingWidth -= column.width;
        --remainingColumns;
        maxWidthPerCol = remainingWidth / std::max(remainingColumns, 1L);
        ++idx;
    }

    if (!columnsRef.empty() && remainingWidth > 0 && biggerColumn.second > 0)
    {
        columnsRef[biggerColumn.first].get().width += remainingWidth;
    }
}

void LogViewWidget::adjustColumns(ColumnsSize size)
{
    tp::ColumnsRef headerColumns;
    m_header->getVisibleColumns(headerColumns);

    switch (size)
    {
        case ColumnsSize::Headers:
            getColumnsSizeToHeader(headerColumns);
            break;
        case ColumnsSize::Content:
            getColumnsSizeToContent(headerColumns);
            break;
        case ColumnsSize::Screen:
            getColumnsSizeToScreen(headerColumns);
            break;
        default:
            break;
    }

    m_header->updateColumns();

    m_stabilizedUpdateTimer->start();
}

void LogViewWidget::expandColumnToContent(tp::SInt columnIdx)
{
    if (columnIdx < m_header->getColumns().size())
    {
        tp::ColumnsRef columnsRef;
        columnsRef.push_back(m_header->getColumns().at(columnIdx));
        getColumnsSizeToContent(columnsRef);
        m_header->updateColumns();
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

void LogViewWidget::goToPrevRow()
{
    if (m_currentRow.value_or(0L) < 1L)
        return;
    *m_currentRow -= 1;
    m_vScrollBar->setPos(m_vScrollBar->getPos() - 1);
    update();
}

void LogViewWidget::goToNextRow()
{
    const auto lastRow = m_model->rowCount() - 1;
    if (m_currentRow.value_or(lastRow) >= lastRow)
        return;
    *m_currentRow += 1;
    m_vScrollBar->setPos(m_vScrollBar->getPos() + 1);
    update();
}

void LogViewWidget::goToPrevPage()
{
    m_vScrollBar->setPos(getFirstPageRow() - m_itemsPerPage + 1);
    update();
}

void LogViewWidget::goToNextPage()
{
    m_vScrollBar->setPos(getFirstPageRow() + m_itemsPerPage - 1);
    update();
}

void LogViewWidget::gotToFirstRow()
{
    m_vScrollBar->setPos(0);
    update();
}

void LogViewWidget::gotToLastRow()
{
    m_vScrollBar->setPos(m_vScrollBar->getMax());
    update();
}

void LogViewWidget::goLeft()
{
    const tp::SInt step = m_textAreaRect.width() / 20L;
    m_hScrollBar->setPos(m_hScrollBar->getPos() - step);
    update();
}

void LogViewWidget::goRight()
{
    const tp::SInt step = m_textAreaRect.width() / 20L;
    m_hScrollBar->setPos(m_hScrollBar->getPos() + step);
    update();
}

void LogViewWidget::goFullLeft()
{
    m_hScrollBar->setPos(0);
    update();
}

void LogViewWidget::goFullRight()
{
    m_hScrollBar->setPos(m_hScrollBar->getMax());
    update();
}

bool LogViewWidget::canCopy() const
{
    return ((m_startSelect.has_value() && m_currentSelec.has_value()) || m_currentRow.has_value());
}

QString LogViewWidget::rowToText(tp::SInt row)
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

QString LogViewWidget::getSelectedText(tp::SInt row)
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
            const tp::SInt firstRow = std::min(m_startSelect->first, m_currentSelec->first);
            const tp::SInt lastRow = std::max(m_startSelect->first, m_currentSelec->first);
            for (tp::SInt row = firstRow; row <= lastRow; ++row)
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

const std::set<tp::SInt> &LogViewWidget::getMarks()
{
    return m_marks;
}

bool LogViewWidget::hasMark(tp::SInt row) const
{
    return (m_marks.find(row) != m_marks.end());
}

void LogViewWidget::clearMarks()
{
    m_marks.clear();
}

void LogViewWidget::markRow(tp::SInt row)
{
    m_marks.insert(row);
}

void LogViewWidget::removeMark(tp::SInt row)
{
    if (auto it = m_marks.find(row); it != m_marks.end())
    {
        m_marks.erase(it);
    }
}

void LogViewWidget::toggleMark(tp::SInt row)
{
    if (auto it = m_marks.find(row); it != m_marks.end())
    {
        m_marks.erase(it);
        return;
    }
    m_marks.insert(row);
}

bool LogViewWidget::hasPrevMark()
{
    if (!m_currentRow.has_value())
        return false;
    return (m_marks.lower_bound(m_currentRow.value()) != m_marks.begin());
}

bool LogViewWidget::hasNextMark()
{
    if (!m_currentRow.has_value())
        return false;
    return (m_marks.upper_bound(m_currentRow.value()) != m_marks.end());
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

void LogViewWidget::markSelected()
{
    if (m_startSelect.has_value() && m_currentSelec.has_value())
    {
        const tp::SInt firstRow = std::min(m_startSelect->first, m_currentSelec->first);
        const tp::SInt lastRow = std::max(m_startSelect->first, m_currentSelec->first);
        bool allMarked(true);
        for (tp::SInt row = firstRow; row <= lastRow; ++row)
        {
            if (!hasMark(row))
            {
                allMarked = false;
                break;
            }
        }
        for (tp::SInt row = firstRow; row <= lastRow; ++row)
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