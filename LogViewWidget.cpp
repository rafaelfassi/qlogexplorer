#include "LogViewWidget.h"
#include "LongScrollBar.h"
#include "AbstractModel.h"
#include <QScrollBar>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
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
    m_vScrollBar = new LongScrollBar(Qt::Vertical, this);
    m_vScrollBar->setFixedWidth(gScrollBarThickness);
    m_vScrollBar->sizePolicy().setVerticalPolicy(QSizePolicy::Expanding);
    m_vScrollBar->sizePolicy().setHorizontalPolicy(QSizePolicy::Fixed);

    m_hScrollBar = new LongScrollBar(Qt::Horizontal, this);
    m_hScrollBar->setPosPerStep(20);
    m_hScrollBar->setFixedHeight(gScrollBarThickness);
    m_hScrollBar->sizePolicy().setHorizontalPolicy(QSizePolicy::Expanding);
    m_hScrollBar->sizePolicy().setVerticalPolicy(QSizePolicy::Fixed);

    m_hScrollBarLayout = new QVBoxLayout;
    m_hScrollBarLayout->setMargin(0);
    m_hScrollBarLayout->addWidget(m_hScrollBar, 0, Qt::AlignBottom);

    m_vScrollBarLayout = new QVBoxLayout;
    m_vScrollBarLayout->setMargin(0);
    m_vScrollBarLayout->addWidget(m_vScrollBar, 1, Qt::AlignRight);
    m_vScrollBarLayout->addSpacing(gScrollBarThickness);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    hLayout->addLayout(m_hScrollBarLayout, 1);
    hLayout->addLayout(m_vScrollBarLayout);

    setLayout(hLayout);

    m_updateTimer = new QTimer(this);

    connect(m_vScrollBar, &LongScrollBar::posChanged, this, qOverload<>(&QWidget::update));
    connect(m_hScrollBar, &LongScrollBar::posChanged, this, qOverload<>(&QWidget::update));
    connect(m_updateTimer, &QTimer::timeout, this, qOverload<>(&LogViewWidget::updateView));
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
        fillColumns();
        m_rowHeight = m_fm.height();
        m_itemsPerPage = m_textAreaRect.height() / m_rowHeight;

        m_vScrollBar->setMax(rowCount - m_itemsPerPage);
        m_hScrollBar->setMax(m_totalWidthInPage - m_textAreaRect.width());

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
    m_textAreaRect.setHeight(height() - gScrollBarThickness - m_textAreaRect.top());
    m_textAreaRect.setWidth(width() - gScrollBarThickness - m_textAreaRect.left());
    updateDisplaySize();
}

void LogViewWidget::paintEvent(QPaintEvent *event)
{
    const QRect invalidRect = event->rect();
    if (invalidRect.isEmpty() || (m_logModel == nullptr))
        return;

    m_updateTimer->stop();

    QPainter devicePainter(this);
    devicePainter.setFont(m_font);
    devicePainter.eraseRect(rect());

    const ssize_t modelRowCount(m_logModel->rowCount());
    ssize_t hScrollOffset(m_hScrollBar->getPos());
    ssize_t rowsToRender(std::min<ssize_t>(m_itemsPerPage, modelRowCount));

    // Draw Header
    {
        QRect rect(m_textAreaRect.left(), 0, m_textAreaRect.width(), m_textAreaRect.top());
        devicePainter.setClipRect(rect);
        rect.moveLeft(m_textAreaRect.left() - hScrollOffset);
        rect.setWidth(m_textAreaRect.width() + hScrollOffset);
        devicePainter.fillRect(rect, QColor("#5f9ea0"));
        for (auto &column : m_columns)
        {
            rect.setWidth(column.width);
            devicePainter.drawText(rect, Qt::AlignTop | Qt::AlignLeft, column.name.c_str());
            rect.moveLeft(rect.left() + rect.width() + gDefaultMarging);
            column.maxWidth = getTextWidth(column.name);
        }
    }

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

            for (auto &column : m_columns)
            {
                rect.setWidth(column.width);
                if (column.modelIdx < rowData.size())
                {
                    QString colText;
                    ssize_t txtWidth = getTextWidth(rowData[column.modelIdx], &colText);
                    column.maxWidth = std::max<ssize_t>(column.maxWidth, txtWidth);
                    devicePainter.drawText(rect, Qt::AlignTop | Qt::AlignLeft, colText);
                }
                rect.setLeft(rect.left() + column.width + gDefaultMarging);
            }
        }
    }

    bool mustUpdate(false);
    ssize_t totalWidthInPage(0);
    for (auto &column : m_columns)
    {
        if (!column.fixedSize && (column.width != column.maxWidth))
        {
            column.width = column.maxWidth;
            mustUpdate = true;
        }
        totalWidthInPage += column.width + gDefaultMarging;
    }

    if (m_totalWidthInPage != totalWidthInPage)
    {
        m_totalWidthInPage = totalWidthInPage;
        mustUpdate = true;
    }

    if (mustUpdate)
    {
        m_updateTimer->start(200);
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
        std::swap(m_columns[0], m_columns[1]);
        qDebug() << "Header clicked";
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

ssize_t LogViewWidget::getTextWidth(const std::string &text, QString *qStrText)
{
    QString tmpStr;
    if (qStrText == nullptr)
    {
        qStrText = &tmpStr;
    }

    *qStrText = QString::fromStdString(text).simplified();
    return m_fm.horizontalAdvance(*qStrText);
}

void LogViewWidget::fillColumns()
{
    if (m_columns.empty() && (m_logModel != nullptr) && !m_logModel->getColumns().empty())
    {
        ssize_t defColWidth(m_textAreaRect.width() / m_logModel->getColumns().size());
        ssize_t colIdx(0);
        for (const auto &columnName : m_logModel->getColumns())
        {
            TableColumn tbCol;
            tbCol.modelIdx = colIdx++;
            tbCol.name = columnName;
            tbCol.width = std::max<ssize_t>(getTextWidth(columnName) + gDefaultMarging, defColWidth);
            tbCol.maxWidth = tbCol.maxWidth;
            m_columns.emplace_back(std::move(tbCol));
        }

        if ((m_logModel->getColumns().size() > 0) && !m_logModel->getColumns().front().empty())
        {
            m_textAreaRect.setTop(m_fm.height());
            m_vScrollBarLayout->setContentsMargins(0, m_textAreaRect.top(), 0, 0);
        }
        else
        {
            m_textAreaRect.setTop(0);
        }
    }
}