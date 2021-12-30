#include "LogView.h"
#include "AbstractLogModel.h"
#include <QScrollBar>
#include <QPaintEvent>
#include <QPainter>
#include <cmath>

LogView::LogView(QWidget *parent)
    : QAbstractScrollArea(parent),
    m_font("times", 14),
    m_fm(m_font),
    m_rowHeight(0),
    m_itemsPerPage(0)
{

}

LogView::~LogView()
{

}

void LogView::setLogModel(AbstractLogModel *logModel)
{
    m_logModel = logModel;
    updateDisplaySize();
    connect(logModel, &AbstractLogModel::countChanged, this, &LogView::updateDisplaySize);
}

void LogView::updateDisplaySize()
{
    if (m_logModel)
    {
        m_rowHeight = m_fm.height();
        m_itemsPerPage =  (height() - m_rowHeight/2) / m_rowHeight;
        verticalScrollBar()->setPageStep(1);
        verticalScrollBar()->setRange(0, m_logModel->rowCount() - m_itemsPerPage);
    }
    else
    {
        verticalScrollBar()->setRange(0, 0);
    }
}

void LogView::resizeEvent(QResizeEvent *event)
{
    updateDisplaySize();
}

void LogView::paintEvent(QPaintEvent *event)
{
    const QRect invalidRect = event->rect();
    if (invalidRect.isEmpty() || (m_logModel == nullptr))
        return;


    QPainter devicePainter( viewport() );
    devicePainter.setFont(m_font);
    devicePainter.eraseRect( rect() );
    //devicePainter.drawLine(0, 0, 100, 100);
    std::vector<std::string> rowData;

    int leftMargin(10);

    for(int i = 0; i < m_itemsPerPage; ++i)
    {
        rowData.clear();
        m_logModel->getRow(i + verticalScrollBar()->value(), rowData);
        int textWidth(leftMargin);
        for(const auto& col : rowData)
        {
            const auto& colText = QString::fromStdString(col);
            devicePainter.drawText(textWidth, m_fm.height()+(m_rowHeight*i), colText);
            textWidth += leftMargin + m_fm.horizontalAdvance(colText);
        }
    }

}

