#include "LogViewWidget.h"
#include "LongScrollBar.h"
#include "AbstractLogModel.h"
#include <QScrollBar>
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <cmath>
#include <limits>
#include <iostream>

constexpr int gScrollBarThickness(25);

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

    QVBoxLayout *vLayout1 = new QVBoxLayout;
    vLayout1->setMargin(0);
    vLayout1->addWidget(m_hScrollBar, 0, Qt::AlignBottom);

    QVBoxLayout *vLayout2 = new QVBoxLayout;
    vLayout2->setMargin(0);
    vLayout2->addWidget(m_vScrollBar, 1, Qt::AlignRight);
    vLayout2->addSpacing(gScrollBarThickness);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    hLayout->addLayout(vLayout1, 1);
    hLayout->addLayout(vLayout2);

    setLayout(hLayout);

    connect(m_vScrollBar, &LongScrollBar::posChanged, this, qOverload<>(&QWidget::update));
    connect(m_hScrollBar, &LongScrollBar::posChanged, this, qOverload<>(&QWidget::update));
}

LogViewWidget::~LogViewWidget()
{
}

void LogViewWidget::setLogModel(AbstractLogModel *logModel)
{
    m_logModel = logModel;
    modelCountChanged();
    connect(logModel, &AbstractLogModel::countChanged, this, &LogViewWidget::modelCountChanged, Qt::UniqueConnection);
}

void LogViewWidget::modelCountChanged()
{
    updateDisplaySize();
    update();
}

void LogViewWidget::updateDisplaySize()
{
    if (m_logModel)
    {
        m_rowHeight = m_fm.height();
        m_itemsPerPage = m_textAreaRect.height() / m_rowHeight;

        m_vScrollBar->setMax(m_logModel->rowCount() - m_itemsPerPage);
        m_hScrollBar->setMax(m_biggerTextWidthInPage - m_textAreaRect.width());
    }
    else
    {
        m_vScrollBar->setMax(-1);
        m_hScrollBar->setMax(-1);
    }
}

void LogViewWidget::resizeEvent(QResizeEvent *event)
{
    m_textAreaRect.setHeight(height() - gScrollBarThickness);
    m_textAreaRect.setWidth(width() - gScrollBarThickness);
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
    // devicePainter.drawLine(0, 0, 100, 100);
    std::vector<std::string> rowData;

    int leftMargin(10);

    int lineNumberWidth = m_fm.horizontalAdvance(QString::number(m_logModel->rowCount()));

    ssize_t hScrollOffset(m_hScrollBar->getPos());

    ssize_t rowsToRender = std::min<ssize_t>(m_itemsPerPage, m_logModel->rowCount());

    m_biggerTextWidthInPage = 0;

    for (ssize_t i = 0; i < rowsToRender; ++i)
    {
        ssize_t row = i + m_vScrollBar->getPos();
        int rY = m_rowHeight * i;
        rowData.clear();
        m_logModel->getRow(row, rowData);
        int textWidth(leftMargin);
        if (m_currentRow.has_value() && m_currentRow.value() == row)
        {
            devicePainter.fillRect(0, rY, width(), m_fm.height(), QColor("blue"));
        }

        QRect lineRect(textWidth - hScrollOffset, rY, m_textAreaRect.width(), m_textAreaRect.height());
        devicePainter.drawText(lineRect, Qt::AlignTop | Qt::AlignLeft, std::to_string(row).c_str());
        textWidth += leftMargin + lineNumberWidth;

        for (const auto &col : rowData)
        {
            QRect lineRect(
                textWidth - hScrollOffset,
                rY,
                m_textAreaRect.width() + hScrollOffset,
                m_textAreaRect.height());
            const auto &colText = QString::fromStdString(col);
            // devicePainter.drawText(textWidth, rY, colText);
            devicePainter.drawText(lineRect, Qt::AlignTop | Qt::AlignLeft, colText);
            textWidth += leftMargin + m_fm.horizontalAdvance(colText);
        }

        m_biggerTextWidthInPage = std::max(textWidth, m_biggerTextWidthInPage);

        if (textWidth - m_textAreaRect.width() > m_hScrollBar->getMax())
        {
            m_hScrollBar->setMax(textWidth - m_textAreaRect.width());
        }
    }
}

void LogViewWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_logModel == nullptr)
    {
        return;
    }
    const auto row = m_vScrollBar->getPos() + (event->pos().y() / m_rowHeight);
    if (row < m_logModel->rowCount())
    {
        m_currentRow = row;
        update();
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