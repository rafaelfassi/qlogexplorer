#include "HeaderView.h"
#include "LogViewWidget.h"
#include "LongScrollBar.h"
#include "AbstractModel.h"
#include <QScrollBar>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QPushButton>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <cmath>
#include <limits>
#include <iostream>

constexpr int gScrollBarThickness(25);
constexpr ssize_t gDefaultMarging(10);

LogViewWidget::LogViewWidget(QWidget *parent) : QWidget(parent), m_font("times", 14), m_fm(m_font)
{
    m_header = new HeaderView(this);
    m_header->setFont(&m_font);
    m_header->setMaximumHeight(m_fm.height());
    m_header->setFixedHeight(m_header->maximumHeight());
    m_header->sizePolicy().setHorizontalPolicy(QSizePolicy::Expanding);
    m_header->sizePolicy().setVerticalPolicy(QSizePolicy::Fixed);

    m_vScrollBar = new LongScrollBar(Qt::Vertical, this);
    m_vScrollBar->setFixedWidth(gScrollBarThickness);
    m_vScrollBar->sizePolicy().setVerticalPolicy(QSizePolicy::Expanding);
    m_vScrollBar->sizePolicy().setHorizontalPolicy(QSizePolicy::Fixed);

    m_hScrollBar = new LongScrollBar(Qt::Horizontal, this);
    m_hScrollBar->setPosPerStep(20);
    m_hScrollBar->setFixedHeight(gScrollBarThickness);
    m_hScrollBar->sizePolicy().setHorizontalPolicy(QSizePolicy::Expanding);
    m_hScrollBar->sizePolicy().setVerticalPolicy(QSizePolicy::Fixed);

    m_btnExpandColumns = new QPushButton(this);
    m_btnExpandColumns->setFocusPolicy(Qt::NoFocus);
    m_btnExpandColumns->setIcon(QIcon(":/images/expand_icon.png"));
    m_btnExpandColumns->setToolTip("Expand All Columns");
    m_btnExpandColumns->setFlat(true);
    m_btnExpandColumns->setFixedHeight(m_header->maximumHeight());
    m_btnExpandColumns->setFixedWidth(gScrollBarThickness);

    m_btnFitColumns = new QPushButton(this);
    m_btnFitColumns->setFocusPolicy(Qt::NoFocus);
    m_btnFitColumns->setIcon(QIcon(":/images/fit_icon.png"));
    m_btnFitColumns->setToolTip("Adjust Columns to Fit");
    m_btnFitColumns->setFlat(true);
    m_btnFitColumns->setFixedHeight(gScrollBarThickness);
    m_btnFitColumns->setFixedWidth(gScrollBarThickness);

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

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(200);

    connect(m_vScrollBar, &LongScrollBar::posChanged, this, &LogViewWidget::vScrollBarPosChanged);
    connect(m_hScrollBar, &LongScrollBar::posChanged, this, &LogViewWidget::hScrollBarPosChanged);
    connect(m_updateTimer, &QTimer::timeout, this, &LogViewWidget::updateRowWidth);
    connect(m_header, &QHeaderView::geometriesChanged, this, &LogViewWidget::headerChanged);
    connect(m_header, &QHeaderView::sectionResized, this, [this](int, int, int) { this->headerChanged(); });
    connect(m_header, &QHeaderView::sectionMoved, this, [this](int, int, int) { this->headerChanged(); });
    connect(m_header, &HeaderView::expandToContent, this, &LogViewWidget::expandColumnToContent);
    connect(m_header, &HeaderView::expandAllToContent, this, [this]() { this->adjustColumns(ColumnsSize::Content); });
    connect(m_header, &HeaderView::expandAllToScreen, this, [this]() { this->adjustColumns(ColumnsSize::Screen); });
    connect(m_btnExpandColumns, &QPushButton::clicked, this, [this]() { this->adjustColumns(ColumnsSize::Content); });
    connect(m_btnFitColumns, &QPushButton::clicked, this, [this]() { this->adjustColumns(ColumnsSize::Screen); });
}

LogViewWidget::~LogViewWidget()
{
}

void LogViewWidget::setLogModel(AbstractModel *logModel)
{
    m_logModel = logModel;
    modelCountChanged();
    connect(logModel, &AbstractModel::countChanged, this, &LogViewWidget::modelCountChanged, Qt::UniqueConnection);
}

void LogViewWidget::headerChanged()
{
    updateView();
    m_updateTimer->start();
}

void LogViewWidget::updateRowWidth()
{
    m_updateTimer->stop();
    disconnect(m_header, &QHeaderView::sectionResized, nullptr, nullptr);

    if (m_header->isVisible())
    {
        ssize_t totalWidth(0);
        for (size_t visualIdx = 0; visualIdx < m_header->count(); ++visualIdx)
        {
            const ssize_t logicalIdx = m_header->logicalIndex(visualIdx);
            totalWidth += m_header->sectionSize(logicalIdx);
        }
        m_rowWidth = totalWidth;
    }

    if (!m_vScrollBar->isKnobGrabbed() && !m_hScrollBar->isKnobGrabbed())
    {
        m_hScrollBar->setMax(m_rowWidth - m_textAreaRect.width());
    }
    else
    {
        m_updateTimer->start();
    }

    connect(m_header, &QHeaderView::sectionResized, this, [this](int, int, int) { this->headerChanged(); });
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
    if ((m_logModel != nullptr) && (row >= 0) && (row < m_logModel->rowCount()))
    {
        if ((row < m_vScrollBar->getPos()) || ((row - m_vScrollBar->getPos()) >= m_itemsPerPage))
        {
            m_vScrollBar->setPos(row - (m_itemsPerPage / 2));
        }

        m_currentRow = row;
        update();
    }
}

void LogViewWidget::updateDisplaySize()
{
    if (m_logModel)
    {
        ssize_t rowCount(m_logModel->rowCount());
        configureColumns();
        m_rowHeight = m_fm.height();
        m_itemsPerPage = m_textAreaRect.height() / m_rowHeight;

        m_vScrollBar->setMax(rowCount - m_itemsPerPage);

        // qDebug() << "     width" << m_textAreaRect.width();
        // qDebug() << "m_rowWidth" << m_rowWidth;

        const std::string lastLineNumberStr(std::to_string(m_logModel->getRowNum(rowCount - 1L) + 1L));
        m_textAreaRect.setLeft(getTextWidth(lastLineNumberStr) + gDefaultMarging);
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
    if (m_header->isVisible())
    {
        m_textAreaRect.setTop(m_header->height());
    }

    m_textAreaRect.setHeight(height() - gScrollBarThickness - m_textAreaRect.top());
    m_textAreaRect.setWidth(width() - gScrollBarThickness - m_textAreaRect.left());
    updateRowWidth();
    updateDisplaySize();
}

void LogViewWidget::paintEvent(QPaintEvent *event)
{
    const QRect invalidRect = event->rect();
    if (invalidRect.isEmpty() || (m_logModel == nullptr))
        return;

    QPainter devicePainter(this);
    devicePainter.setFont(m_font);
    devicePainter.eraseRect(rect());

    const ssize_t modelRowCount(m_logModel->rowCount());
    ssize_t hScrollOffset(m_hScrollBar->getPos());
    ssize_t rowsToRender(std::min<ssize_t>(m_itemsPerPage, modelRowCount));
    ssize_t maxRowWidth(0);

    std::vector<std::string> rowData;
    for (ssize_t i = 0; i < rowsToRender; ++i)
    {
        ssize_t row = i + m_vScrollBar->getPos();
        int rY = m_textAreaRect.top() + (m_rowHeight * i);
        rowData.clear();
        ssize_t rowNumb = m_logModel->getRow(row, rowData);

        devicePainter.setPen(Qt::black);

        // Draw Line Number
        {
            devicePainter.setClipping(false);
            QRect rect(0, rY, m_textAreaRect.left(), m_rowHeight);
            devicePainter.fillRect(rect, QColor("#ffebcd"));
            devicePainter.drawText(rect, Qt::AlignTop | Qt::AlignLeft, std::to_string(rowNumb).c_str());
        }

        // Draw Row Data
        {
            devicePainter.setClipRect(m_textAreaRect);
            QRect rect(m_textAreaRect.left() - hScrollOffset, rY, m_textAreaRect.width() + hScrollOffset, m_rowHeight);

            // Highlight if selected
            if (m_currentRow.has_value() && m_currentRow.value() == row)
            {
                devicePainter.fillRect(rect, QColor("#4169e1"));
                devicePainter.setPen(Qt::white);
            }

            if (m_header->isVisible())
            {
                for (size_t visualIdx = 0; visualIdx < m_header->count(); ++visualIdx)
                {
                    const ssize_t logicalIdx = m_header->logicalIndex(visualIdx);
                    const ssize_t colWidth = m_header->sectionSize(logicalIdx);
                    rect.setWidth(colWidth);
                    if (logicalIdx < rowData.size())
                    {
                        const QString &colText(getElidedText(rowData[logicalIdx], colWidth - gDefaultMarging, true));
                        devicePainter.drawText(rect, Qt::AlignTop | Qt::AlignLeft, colText);
                    }
                    rect.moveLeft(rect.left() + rect.width());
                }
            }
            else
            {
                ssize_t rowWidth(0);
                for (const auto &colText : rowData)
                {
                    const ssize_t colWidth = getTextWidth(colText) + gDefaultMarging;
                    rect.setWidth(colWidth);
                    devicePainter.drawText(
                        rect,
                        Qt::AlignTop | Qt::AlignLeft,
                        QString::fromStdString(colText).simplified());
                    rect.moveLeft(rect.left() + rect.width());
                    rowWidth += colWidth;
                }
                maxRowWidth = std::max(maxRowWidth, rowWidth);
            }
        }
    }

    if (!m_header->isVisible() && (maxRowWidth != m_rowWidth))
    {
        m_rowWidth = maxRowWidth;
        m_updateTimer->start();
    }
}

void LogViewWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_logModel == nullptr)
    {
        return;
    }

    if (event->pos().y() <= m_textAreaRect.top())
    {
        if (event->modifiers() == Qt::ShiftModifier)
        {
            m_header->hideSection(1);
            m_header->hideSection(4);
        }
        else
        {
            adjustColumns(ColumnsSize::Content);
        }
        return;
    }

    const auto row = m_vScrollBar->getPos() + ((event->pos().y() - m_textAreaRect.top()) / m_rowHeight);
    if (row >= 0 && (row < m_logModel->rowCount()))
    {
        m_currentRow = row;
        update();
        emit rowSelected(m_logModel->getRowNum(row));
    }
}

void LogViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
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
    if ((m_header->count() == 0) && !m_logModel->getColumns().empty())
    {
        m_header->setColumns(m_logModel->getColumns());
        if (m_logModel->getColumns().front().empty())
        {
            m_header->setVisible(false);
            m_btnExpandColumns->setVisible(false);
            m_btnFitColumns->setEnabled(false);
            m_btnFitColumns->setIcon(QIcon());
            m_btnFitColumns->setToolTip("");
        }
        else
        {
            adjustColumns(ColumnsSize::Screen);
        }
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
            const ssize_t headerTextWidth = getTextWidth(headerText) + gDefaultMarging;
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

    ssize_t rowsInPage(std::min<ssize_t>(m_itemsPerPage, m_logModel->rowCount()));
    const ssize_t elideWith(getTextWidth("..."));
    std::vector<std::string> rowData;

    for (ssize_t i = 0; i < rowsInPage; ++i)
    {
        ssize_t row = i + m_vScrollBar->getPos();
        rowData.clear();
        m_logModel->getRow(row, rowData);

        for (auto &[idx, columnWidth] : columnSizesMap)
        {
            const ssize_t textWidth = getTextWidth(rowData[idx], true) + elideWith + gDefaultMarging;
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

    m_updateTimer->start();
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