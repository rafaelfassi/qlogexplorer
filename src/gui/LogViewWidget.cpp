// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "Style.h"
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

LogViewWidget::LogViewWidget(AbstractModel *model, QWidget *parent) : QWidget(parent), m_model(model)
{
    auto pal = palette();
    pal.setColor(QPalette::Window, Style::getHeaderColor().bg);
    setPalette(pal);

    m_header = new HeaderView(this);
    m_header->setMaximumHeight(Style::getTextHeight(true));
    m_header->setFixedHeight(m_header->maximumHeight());
    auto headerPolicy = m_header->sizePolicy();
    headerPolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    headerPolicy.setVerticalPolicy(QSizePolicy::Fixed);
    m_header->setSizePolicy(headerPolicy);

    m_vScrollBar = new LongScrollBar(Qt::Vertical, this);
    m_vScrollBar->setFixedWidth(Style::getScrollBarThickness());
    auto vScrollBarPolicy = m_vScrollBar->sizePolicy();
    vScrollBarPolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    vScrollBarPolicy.setVerticalPolicy(QSizePolicy::Expanding);
    m_vScrollBar->setSizePolicy(vScrollBarPolicy);

    m_hScrollBar = new LongScrollBar(Qt::Horizontal, this);
    m_hScrollBar->setPosPerStep(20);
    m_hScrollBar->setFixedHeight(Style::getScrollBarThickness());
    auto hScrollBarPolicy = m_hScrollBar->sizePolicy();
    hScrollBarPolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    hScrollBarPolicy.setVerticalPolicy(QSizePolicy::Fixed);
    m_hScrollBar->setSizePolicy(hScrollBarPolicy);

    m_btnExpandColumns = new QPushButton(this);
    m_btnExpandColumns->setFocusPolicy(Qt::NoFocus);
    m_btnExpandColumns->setIcon(Style::getIcon("expand_icon.png"));
    m_btnExpandColumns->setToolTip("Expand All Columns");
    m_btnExpandColumns->setFlat(true);
    m_btnExpandColumns->setFixedHeight(m_header->maximumHeight());
    m_btnExpandColumns->setFixedWidth(Style::getScrollBarThickness());

    m_btnFitColumns = new QPushButton(this);
    m_btnFitColumns->setFocusPolicy(Qt::NoFocus);
    m_btnFitColumns->setIcon(Style::getIcon("fit_icon.png"));
    m_btnFitColumns->setToolTip("Adjust Columns to Fit");
    m_btnFitColumns->setFlat(true);
    m_btnFitColumns->setFixedHeight(Style::getScrollBarThickness());
    m_btnFitColumns->setFixedWidth(Style::getScrollBarThickness());

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

    m_actBookmark = new QAction("Toggle Bookmark", this);
    m_actBookmark->setShortcut(Qt::CTRL + Qt::Key_M);
    m_actBookmark->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actBookmark);

    m_actPrevBookmark = new QAction("Previous Bookmark", this);
    m_actPrevBookmark->setShortcut(Qt::CTRL + Qt::Key_Up);
    m_actPrevBookmark->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actPrevBookmark);

    m_actNextBookmark = new QAction("Next Bookmark", this);
    m_actNextBookmark->setShortcut(Qt::CTRL + Qt::Key_Down);
    m_actNextBookmark->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actNextBookmark);

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    connect(m_model, &AbstractModel::modelConfigured, this, &LogViewWidget::configureColumns, Qt::QueuedConnection);
    connect(m_model, &AbstractModel::countChanged, this, &LogViewWidget::modelCountChanged, Qt::QueuedConnection);
    connect(m_vScrollBar, &LongScrollBar::posChanged, this, &LogViewWidget::vScrollBarPosChanged);
    connect(m_hScrollBar, &LongScrollBar::posChanged, this, &LogViewWidget::hScrollBarPosChanged);
    connect(m_stabilizedUpdateTimer, &QTimer::timeout, this, &LogViewWidget::stabilizedUpdate);
    connect(m_header, &HeaderView::columnsChanged, this, &LogViewWidget::headerChanged);
    connect(m_header, &HeaderView::expandToContent, this, &LogViewWidget::expandColumnToContent);
    connect(m_header, &HeaderView::expandAllToContent, this, [this]() { this->adjustColumns(ColumnsFit::Content); });
    connect(m_header, &HeaderView::expandAllToScreen, this, [this]() { this->adjustColumns(ColumnsFit::Screen); });
    connect(m_btnExpandColumns, &QPushButton::clicked, this, [this]() { this->adjustColumns(ColumnsFit::Content); });
    connect(m_btnFitColumns, &QPushButton::clicked, this, [this]() { this->adjustColumns(ColumnsFit::Screen); });
    connect(m_actCopy, &QAction::triggered, this, &LogViewWidget::copySelected);
    connect(m_actBookmark, &QAction::triggered, this, &LogViewWidget::bookmarkSelected);
    connect(m_actPrevBookmark, &QAction::triggered, this, &LogViewWidget::goToPrevBookmark);
    connect(m_actNextBookmark, &QAction::triggered, this, &LogViewWidget::goToNextBookmark);
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

    m_availableMarks.emplace_back("#000000", "#7fffd4");
    m_availableMarks.emplace_back("#000000", "#87cefa");
    m_availableMarks.emplace_back("#000000", "#fff8dc");
    m_availableMarks.emplace_back("#ffffff", "#9932cc");
    m_availableMarks.emplace_back("#ffffff", "#ff8c00");
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

        m_selectedRow = row;

        if (m_selectStart.has_value() || m_selectEnd.has_value())
        {
            m_selectStart = std::nullopt;
            m_selectEnd = std::nullopt;
        }

        update();
    }
}

void LogViewWidget::updateDisplaySize()
{
    if (m_model)
    {
        tp::SInt rowCount(m_model->rowCount());
        m_rowHeight = Style::getTextHeight(false);
        m_itemsPerPage = m_textAreaRect.height() / m_rowHeight;

        m_vScrollBar->setMax(rowCount - m_itemsPerPage);

        const std::string lastLineNumberStr(std::to_string(m_model->getRowNum(rowCount - 1L) + 1L));
        m_textAreaRect.setLeft(getTextWidth(lastLineNumberStr) + 2 * Style::getTextPadding());
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
    m_textAreaRect.setHeight(height() - Style::getScrollBarThickness() - m_textAreaRect.top());
    m_textAreaRect.setWidth(width() - Style::getScrollBarThickness() - m_textAreaRect.left());
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
        if (m_selectStart.has_value() && (event->modifiers() == Qt::ShiftModifier))
        {
            // Selects many rows by clicking with shift.
            m_selectEnd = std::make_pair(row, xPos + m_hScrollBar->getPos());
        }
        else
        {
            m_selectStart = std::make_pair(row, xPos + m_hScrollBar->getPos());
            m_selectEnd = std::nullopt;
            m_selectedText = std::nullopt;
            m_selectedRow = row;
            emit rowSelected(m_model->getRowNum(row));
        }
        update();
    }
    else
    {
        if (!m_selectEnd.has_value() && (m_selectedRow != row))
        {
            m_selectedRow = row;
            emit rowSelected(m_model->getRowNum(row));
        }

        // As an alternative implementation, it could not include the not available actions into the menu.
        m_actCopy->setEnabled(canCopy());
        m_actPrevBookmark->setEnabled(hasPrevBookmark());
        m_actNextBookmark->setEnabled(hasNextBookmark());

        QMenu menu(this);
        menu.addAction(m_actCopy);
        menu.addSeparator();
        menu.addAction(m_actBookmark);
        menu.addAction(m_actPrevBookmark);
        menu.addAction(m_actNextBookmark);

        menu.addSeparator();

        auto markAllMenu = menu.addMenu("Mark All");
        markAllMenu->setEnabled(hasTextSelected());

        auto unmarkAllMenu = menu.addMenu("Unmark All");
        unmarkAllMenu->setEnabled(!m_markedTexts.empty());

        int markCnt(0);
        for (const auto &mark : m_availableMarks)
        {
            QIcon icon(Style::makeIcon(mark.bg));
            ++markCnt;

            if (markAllMenu->isEnabled() && m_selectedText.has_value())
            {
                auto actMark = markAllMenu->addAction(icon, QString("Using style %1").arg(markCnt));
                connect(actMark, &QAction::triggered, [this, &mark]() { this->addTextMark(*m_selectedText, mark); });
            }

            if (unmarkAllMenu->isEnabled())
            {
                const auto it = std::find_if(
                    m_markedTexts.begin(),
                    m_markedTexts.end(),
                    [&mark](const tp::TextSelection &sel) { return sel.color.bg == mark.bg; });
                if (it != m_markedTexts.end())
                {
                    auto actUnmark = unmarkAllMenu->addAction(icon, QString("Style %1").arg(markCnt));
                    connect(actUnmark, &QAction::triggered, [this, &mark]() { this->removeTextMarks(mark); });
                }
            }
        }

        menu.exec(event->globalPos());

        // Let the action enabled for the shortcuts
        m_actCopy->setEnabled(true);
        m_actPrevBookmark->setEnabled(true);
        m_actNextBookmark->setEnabled(true);
    }
}

void LogViewWidget::mouseReleaseEvent(QMouseEvent *)
{
    // if (!m_selectEnd.has_value() && (event->button() == Qt::LeftButton))
    // {
    //     const tp::SInt row = getRowByScreenPos(event->pos().y());
    //     if (row >= 0 && (row < m_model->rowCount()))
    //     {
    //         m_selectedRow = row;
    //         update();
    //         emit rowSelected(m_model->getRowNum(row));
    //     }
    // }
}

void LogViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_selectStart.has_value())
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

        m_selectEnd = std::make_pair(row, xPos + m_hScrollBar->getPos());
        if (m_selectEnd->first == m_selectStart->first)
        {
            m_selectedText = getTextSelection(m_selectEnd->first);
        }

        update();
    }
    else if (m_selectEnd.has_value())
    {
        m_selectEnd = std::nullopt;
        m_selectedText = std::nullopt;
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
        if (!col.can.rect.contains(event->pos()))
            continue;

        const int textXPos = xPos - col.can.rect.left();
        const int chPos = getStrStartPos(textXPos);
        const auto wS =
            std::find_if(col.can.text.rbegin() + col.can.text.size() - chPos, col.can.text.rend(), isSeparator);
        const auto wE = std::find_if(col.can.text.begin() + chPos, col.can.text.end(), isSeparator);
        const auto sPos = std::distance(col.can.text.begin(), wS.base());
        const auto ePos = std::distance(col.can.text.begin(), wE);

        QString selText = col.can.text.mid(sPos, ePos - sPos);
        if (selText.isEmpty())
            break;

        auto can = makeSelCanFromStrPos(col.can, sPos, ePos - sPos);
        can.rect.translate(m_hScrollBar->getPos(), 0);

        m_selectStart = std::make_pair(row, can.rect.left() + getCharMarging());
        m_selectEnd = std::make_pair(row, can.rect.right() - getCharMarging());
        m_selectedText = selText;
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
    painter.setFont(Style::getFont());
    painter.eraseRect(rect());
    painter.setClipRect(m_textAreaRect);
    painter.fillRect(m_textAreaRect, Style::getTextAreaColor().bg);

    forEachVisualRowInPage(
        [&painter, this](VisualRowData &vrData)
        {
            // Draw Line Number
            {
                const auto &lineNumColor =
                    hasBookmark(vrData.row) ? Style::getBookmarkColor() : Style::getHeaderColor();
                painter.setClipping(false);
                painter.setPen(lineNumColor.fg);
                painter.fillRect(vrData.numberAreaRect, lineNumColor.bg);
                painter.drawText(
                    vrData.numberRect,
                    Qt::AlignTop | Qt::AlignRight,
                    std::to_string(vrData.number + 1).c_str());
            }

            QColor rowTextColor(Style::getTextAreaColor().fg);
            painter.setClipping(true);

            // Selected line
            if (vrData.selected)
            {
                painter.fillRect(vrData.rect, Style::getSelectedColor().bg);
                rowTextColor = Style::getSelectedColor().fg;
            }
            else if (vrData.highlighter != nullptr)
            {
                painter.fillRect(vrData.rect, vrData.highlighter->getBgColor());
                rowTextColor = vrData.highlighter->getTextColor();
            }

            painter.setPen(rowTextColor);

            // Draw Row Data
            for (const auto &colData : vrData.columns)
            {
                // Draw column text
                painter.drawText(colData.can.rect, Qt::AlignTop | Qt::AlignLeft, colData.can.text);

                for (const auto &markedText : colData.markedTexts)
                {
                    painter.setClipRect(markedText.can.rect.intersected(m_textAreaRect));
                    painter.fillRect(markedText.can.rect, markedText.color.bg);
                    painter.setPen(markedText.color.fg);
                    painter.drawText(colData.can.rect, Qt::AlignTop | Qt::AlignLeft, colData.can.text);
                    painter.setPen(rowTextColor);
                    painter.setClipRect(m_textAreaRect);
                }

                if (colData.selection.has_value())
                {
                    // Draw selected text
                    const auto &selText = colData.selection.value();
                    painter.setClipRect(selText.can.rect.intersected(m_textAreaRect));
                    painter.fillRect(selText.can.rect, selText.color.bg);
                    painter.setPen(selText.color.fg);
                    painter.drawText(colData.can.rect, Qt::AlignTop | Qt::AlignLeft, colData.can.text);
                    painter.setPen(rowTextColor);
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
    vrData.numberRect =
        QRect(Style::getTextPadding(), yOffset, m_textAreaRect.left() - Style::getTextPadding() * 2, m_rowHeight);

    for (const auto &highlighter : m_highlightersRows)
    {
        if (highlighter.matchInRow(rowData))
        {
            vrData.highlighter = &highlighter;
            break;
        }
    }

    std::optional<QRect> selectText;
    if (m_selectStart.has_value() && m_selectEnd.has_value())
    {
        const auto &[sRow, sPos] = m_selectStart.value();
        const auto &[eRow, ePos] = m_selectEnd.value();
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
    else if (m_selectedRow.has_value() && m_selectedRow.value() == row)
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
            rect.setLeft(rect.left() + Style::getTextPadding());
            vcData.can.rect = rect;
            if (idx < rowData.size())
            {
                const QString &colText(rowData[idx].c_str());
                vcData.can.text = getElidedText(colText, colWidth - Style::getColumnMargin(), true);
                if (selectText.has_value() && rect.contains(selectText.value()))
                {
                    const auto &can = makeSelCanFromSelRect(vcData.can, selectText.value());
                    if (!can.text.isEmpty() && can.rect.isValid())
                    {
                        tp::TextSelection textSelection;
                        textSelection.can = can;
                        textSelection.color = Style::getSelectedColor();
                        vcData.selection = std::move(textSelection);
                    }
                }
                vcData.markedTexts = findMarkedText(tp::TextCan(rect, colText));
            }
            vrData.columns.emplace_back(std::move(vcData));
            rect.moveLeft(rect.left() + rect.width());
        }
    }
    else
    {
        for (const auto &colText : rowData)
        {
            VisualColData vcData;
            const tp::SInt colWidth = getTextWidth(colText) + Style::getColumnMargin();
            rect.setWidth(colWidth);
            rect.setLeft(rect.left() + Style::getTextPadding());
            vcData.can.text = QString::fromStdString(colText);
            vcData.can.rect = rect;
            if (selectText.has_value() && rect.contains(selectText.value()))
            {
                const auto &can = makeSelCanFromSelRect(vcData.can, selectText.value());
                if (!can.text.isEmpty() && can.rect.isValid())
                {
                    tp::TextSelection textSelection;
                    textSelection.can = can;
                    textSelection.color = Style::getSelectedColor();
                    vcData.selection = std::move(textSelection);
                }
            }
            vcData.markedTexts = findMarkedText(vcData.can);
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
                maxRowWidth =
                    std::max<tp::SInt>(maxRowWidth, col.can.rect.right() - vrData.numberAreaRect.right() + offset);
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
    const QString qStr(QString::fromStdString(text));
    return Style::getTextWidth(simplified ? qStr.simplified() : qStr);
}

QString LogViewWidget::getElidedText(const QString &text, tp::SInt width, bool simplified)
{
    return Style::getElidedText(simplified ? text.simplified() : text, width, Qt::ElideRight);
}

void LogViewWidget::configure(FileConf::Ptr conf)
{
    m_highlightersRows.clear();
    for (const auto &param : conf->getHighlighterParams())
    {
        m_highlightersRows.push_back(Highlighter(param));
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
            m_btnFitColumns->setVisible(false);
            m_vScrollBarLayout->setContentsMargins(0, 0, 0, m_btnFitColumns->height());
            m_textAreaRect.setTop(0);
        }
        else
        {
            m_header->setVisible(true);
            m_btnExpandColumns->setVisible(true);
            m_btnFitColumns->setVisible(true);
            m_vScrollBarLayout->setContentsMargins(0, 0, 0, 0);
            m_textAreaRect.setTop(m_header->height());
            adjustColumns(ColumnsFit::Headers);
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
            column.width = getTextWidth(column.name) + Style::getTextPadding() + Style::getColumnMargin();
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
            const tp::SInt textWidth = getTextWidth(rowData[column.idx], true) + Style::getTextPadding() + elideWith +
                                       Style::getColumnMargin();
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
    tp::SInt maxWidthPerCol = remainingWidth / std::max<tp::SInt>(remainingColumns, 1L);
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
        maxWidthPerCol = remainingWidth / std::max<tp::SInt>(remainingColumns, 1L);
        ++idx;
    }

    if (!columnsRef.empty() && remainingWidth > 0 && biggerColumn.second > 0)
    {
        columnsRef[biggerColumn.first].get().width += remainingWidth;
    }
}

void LogViewWidget::adjustColumns(ColumnsFit fit)
{
    tp::ColumnsRef headerColumns;
    m_header->getVisibleColumns(headerColumns);

    switch (fit)
    {
        case ColumnsFit::Headers:
            getColumnsSizeToHeader(headerColumns);
            break;
        case ColumnsFit::Content:
            getColumnsSizeToContent(headerColumns);
            break;
        case ColumnsFit::Screen:
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

qreal LogViewWidget::getCharMarging()
{
    return Style::getCharWidthF() / 4.0;
}

std::vector<tp::TextSelection> LogViewWidget::findMarkedText(const tp::TextCan &can)
{
    std::vector<tp::TextSelection> resVec;

    const auto findAndMarkFunc = [&, this](const QString &text, const tp::SectionColor &color)
    {
        if (!text.isEmpty())
        {
            auto idx = can.text.indexOf(text);
            while (idx != -1)
            {
                const auto &selCan = makeSelCanFromStrPos(can, idx, text.size());

                tp::TextSelection selText;
                selText.can = selCan;
                selText.color = color;
                resVec.emplace_back(std::move(selText));

                idx = can.text.indexOf(text, ++idx);
            }
        }
    };

    for (const auto &markedText : m_markedTexts)
    {
        findAndMarkFunc(markedText.can.text, markedText.color);
    }

    if (m_selectedText.has_value())
    {
        findAndMarkFunc(m_selectedText.value(), Style::getSelectedTextMarkColor());
    }

    return resVec;
}

tp::TextCan LogViewWidget::makeSelCanFromStrPos(const tp::TextCan &can, int fromPos, int len)
{
    int sX = getStrWidthUntilPos(fromPos);
    int eX = getStrWidthUntilPos(fromPos + len);

    tp::TextCan selCan;
    selCan = can;
    selCan.rect.setLeft(selCan.rect.left() + sX);
    selCan.rect.setWidth(eX - sX);
    selCan.rect = selCan.rect.intersected(can.rect);

    return selCan;
}

tp::TextCan LogViewWidget::makeSelCanFromSelRect(const tp::TextCan &can, const QRect &selRect)
{
    tp::TextCan selCan;

    if (can.text.isEmpty() || selRect.isNull() || !selRect.isValid())
    {
        return selCan;
    }

    const int selStart = selRect.left() - can.rect.left();
    const int selEnd = selRect.right() - can.rect.left();

    int sX, eX;
    const int sPos = getStrStartPos(selStart, &sX);
    const int ePos = getStrEndPos(selEnd, &eX, can.text.size());

    if (eX <= sX || ePos <= sPos)
    {
        return selCan;
    }

    selCan.rect = can.rect;
    selCan.rect.setLeft(can.rect.left() + sX);
    selCan.rect.setWidth(eX - sX);
    selCan.text = can.text.mid(sPos, ePos - sPos);
    return selCan;
}

int LogViewWidget::getStrWidthUntilPos(int pos, int maxWidth)
{
    const auto chSize = Style::getCharWidthF();
    int strWidth = chSize * pos;
    if (strWidth > maxWidth)
        return maxWidth / chSize;
    else
        return strWidth;
}

int LogViewWidget::getStrStartPos(int left, int *newLeft)
{
    const auto charWidth = Style::getCharWidthF();
    const int pos = left / charWidth;
    if (newLeft)
        *newLeft = (pos * charWidth);

    return pos;
}

int LogViewWidget::getStrEndPos(int right, int *newRight, int maxSize)
{
    const auto charWidth = Style::getCharWidthF();
    const int pos = std::min<int>((right / charWidth) + 1, maxSize);
    if (newRight)
        *newRight = pos * charWidth;

    return pos;
}

void LogViewWidget::goToPrevRow()
{
    if (m_selectedRow.value_or(0L) < 1L)
        return;
    *m_selectedRow -= 1;
    m_vScrollBar->setPos(m_vScrollBar->getPos() - 1);
    update();
}

void LogViewWidget::goToNextRow()
{
    const auto lastRow = m_model->rowCount() - 1;
    if (m_selectedRow.value_or(lastRow) >= lastRow)
        return;
    *m_selectedRow += 1;
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
    return ((m_selectStart.has_value() && m_selectEnd.has_value()) || m_selectedRow.has_value());
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
        rowText.append(col.can.text);
    }
    return rowText;
}

QString LogViewWidget::getTextSelection(tp::SInt row)
{
    QString textSelection;
    VisualRowData vrData;
    getVisualRowData(row, 0, 0, vrData);
    for (const auto &col : vrData.columns)
    {
        if (col.selection.has_value())
        {
            textSelection = col.selection->can.text;
            break;
        }
    }
    return textSelection;
}

void LogViewWidget::copySelected()
{
    QString value;

    if (m_selectStart.has_value() && m_selectEnd.has_value())
    {
        if (m_selectStart->first == m_selectEnd->first)
        {
            value = getTextSelection(m_selectStart->first);
        }
        else
        {
            const tp::SInt firstRow = std::min(m_selectStart->first, m_selectEnd->first);
            const tp::SInt lastRow = std::max(m_selectStart->first, m_selectEnd->first);
            for (tp::SInt row = firstRow; row <= lastRow; ++row)
            {
                if (!value.isEmpty())
                    value.append('\n');
                value.append(rowToText(row));
            }
        }
    }
    else if (m_selectedRow.has_value())
    {
        value = rowToText(m_selectedRow.value());
    }

    if (!value.isEmpty())
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(value);
    }
}

const std::set<tp::SInt> &LogViewWidget::getBookmarks()
{
    return m_bookMarks;
}

bool LogViewWidget::hasBookmark(tp::SInt row) const
{
    return (m_bookMarks.find(row) != m_bookMarks.end());
}

void LogViewWidget::clearBookmarks()
{
    m_bookMarks.clear();
}

void LogViewWidget::addBookmark(tp::SInt row)
{
    m_bookMarks.insert(row);
}

void LogViewWidget::removeBookmark(tp::SInt row)
{
    if (auto it = m_bookMarks.find(row); it != m_bookMarks.end())
    {
        m_bookMarks.erase(it);
    }
}

void LogViewWidget::toggleBookmark(tp::SInt row)
{
    if (auto it = m_bookMarks.find(row); it != m_bookMarks.end())
    {
        m_bookMarks.erase(it);
        return;
    }
    m_bookMarks.insert(row);
}

bool LogViewWidget::hasPrevBookmark()
{
    if (!m_selectedRow.has_value())
        return false;
    return (m_bookMarks.lower_bound(m_selectedRow.value()) != m_bookMarks.begin());
}

bool LogViewWidget::hasNextBookmark()
{
    if (!m_selectedRow.has_value())
        return false;
    return (m_bookMarks.upper_bound(m_selectedRow.value()) != m_bookMarks.end());
}

void LogViewWidget::goToPrevBookmark()
{
    if (m_selectedRow.has_value())
    {
        auto it = m_bookMarks.lower_bound(m_selectedRow.value());
        if (it != m_bookMarks.begin())
        {
            goToRow(*(--it));
        }
    }
}

void LogViewWidget::goToNextBookmark()
{
    if (m_selectedRow.has_value())
    {
        const auto it = m_bookMarks.upper_bound(m_selectedRow.value());
        if (it != m_bookMarks.end())
        {
            goToRow(*it);
        }
    }
}

void LogViewWidget::bookmarkSelected()
{
    if (m_selectStart.has_value() && m_selectEnd.has_value())
    {
        const tp::SInt firstRow = std::min(m_selectStart->first, m_selectEnd->first);
        const tp::SInt lastRow = std::max(m_selectStart->first, m_selectEnd->first);
        bool allMarked(true);
        for (tp::SInt row = firstRow; row <= lastRow; ++row)
        {
            if (!hasBookmark(row))
            {
                allMarked = false;
                break;
            }
        }
        for (tp::SInt row = firstRow; row <= lastRow; ++row)
        {
            if (allMarked)
                removeBookmark(row);
            else
                addBookmark(row);
        }
    }
    else if (m_selectedRow.has_value())
    {
        toggleBookmark(m_selectedRow.value());
    }

    update();
}

bool LogViewWidget::hasTextSelected()
{
    return (m_selectedText.has_value() && !m_selectedText->isEmpty());
}

void LogViewWidget::addTextMark(const QString &text, const tp::SectionColor &selColor)
{
    m_markedTexts.emplace_back(tp::TextCan(text), selColor);
    update();
}

void LogViewWidget::removeTextMarks(const tp::SectionColor &selColor)
{
    m_markedTexts.erase(
        std::remove_if(
            m_markedTexts.begin(),
            m_markedTexts.end(),
            [&selColor](const tp::TextSelection &mark) { return (mark.color.bg == selColor.bg); }),
        m_markedTexts.end());
    update();
}