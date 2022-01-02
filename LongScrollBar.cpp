#include "LongScrollBar.h"
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <cmath>
#include <limits>
#include <iostream>

LongScrollBar::LongScrollBar(Qt::Orientation orientation, QWidget *parent) : QWidget(parent), m_orientation(orientation)
{
    m_knobRect.setLeft(0);
    m_knobRect.setTop(0);
}

LongScrollBar::~LongScrollBar()
{
}

void LongScrollBar::setMax(ssize_t max)
{
    if (max != m_max)
    {
        changeMax(max);
        if (m_max > 0)
        {
            updateDisplaySize();
            updateKnob();
        }
    }
}

ssize_t LongScrollBar::getMax() const
{
    return m_max;
}

void LongScrollBar::setPos(ssize_t pos)
{
    if (pos != m_pos)
    {
        changePos(pos);
        updateKnob();
    }
}

ssize_t LongScrollBar::getPos() const
{
    return m_pos;
}

void LongScrollBar::setPosPerStep(int positions)
{
    m_posPerStep = positions;
}

void LongScrollBar::updateDisplaySize()
{
    if (m_max <= 0)
    {
        return;
    }

    if (m_orientation == Qt::Vertical)
    {
        updateDisplaySizeV();
    }
    else
    {
        updateDisplaySizeH();
    }
}

void LongScrollBar::updateDisplaySizeV()
{
    const double newHeight(height());
    const double maxPos(std::max(m_max, 1L));

    // qDebug() << "height" << height();

    m_sizePerPos = newHeight / maxPos;
    int knobHeight = std::min(std::max<double>(m_sizePerPos, newHeight / 8.0), newHeight / 2.0);
    // qDebug() << "knobHeight" << knobHeight;

    m_sizePerPos = (newHeight - knobHeight) / maxPos;
    // qDebug() << "m_sizePerPos" << m_sizePerPos;

    m_knobRect.setHeight(knobHeight);
    m_knobRect.setWidth(width());
}

void LongScrollBar::updateDisplaySizeH()
{
    const double newWidth(width());
    const double maxPos(std::max(m_max, 1L));

    // qDebug() << "height" << height();

    m_sizePerPos = newWidth / maxPos;
    int knobWidth = std::min(std::max<double>(m_sizePerPos, newWidth / 8.0), newWidth / 2.0);
    // qDebug() << "knobWidth" << knobWidth;

    m_sizePerPos = (newWidth - knobWidth) / maxPos;
    // qDebug() << "m_sizePerPos" << m_sizePerPos;

    m_knobRect.setWidth(knobWidth);
    m_knobRect.setHeight(height());
}

void LongScrollBar::resizeEvent(QResizeEvent *event)
{
    updateDisplaySize();
    updateKnob();
}

void LongScrollBar::paintEvent(QPaintEvent *event)
{
    const QRect invalidRect = event->rect();
    if (invalidRect.isEmpty())
        return;

    QPainter devicePainter(this);
    devicePainter.eraseRect(rect());

    // qDebug() << "paintEvent: m_knobRect" << m_knobRect;

    devicePainter.fillRect(rect(), QColor("lightGray"));
    if (m_max > 0)
    {
        devicePainter.fillRect(m_knobRect, QColor("darkGray"));
    }
}

void LongScrollBar::mousePressEvent(QMouseEvent *event)
{
    if (m_knobRect.contains(event->pos()))
    {
        if (m_orientation == Qt::Vertical)
        {
            m_knobGrabbed = event->pos().y() - m_knobRect.top();
        }
        else
        {
            m_knobGrabbed = event->pos().x() - m_knobRect.left();
        }
    }
    else
    {
        m_knobGrabbed = std::nullopt;
    }
    // m_knobGrabbed = m_knobRect.contains(event->pos());
    // qDebug() << "mousePressEvent: m_knobGrabbed" << m_knobGrabbed;
}

void LongScrollBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_knobGrabbed.has_value())
    {
        m_knobGrabbed = std::nullopt;
        // qDebug() << "mouseReleaseEvent: m_knobGrabbed" << m_knobGrabbed;
    }
}

void LongScrollBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_knobGrabbed.has_value())
    {
        ssize_t newPos(0);
        if (m_orientation == Qt::Vertical)
        {
            const int newY =
                std::min(std::max(event->pos().y() - m_knobGrabbed.value(), 0), height() - m_knobRect.height());
            newPos = newY / m_sizePerPos;
        }
        else
        {
            const int newX =
                std::min(std::max(event->pos().x() - m_knobGrabbed.value(), 0), width() - m_knobRect.width());
            newPos = newX / m_sizePerPos;
        }

        changePos(std::min<ssize_t>(newPos, m_max));
        updateKnob();
        emit posChanged();
    }
}

void LongScrollBar::wheelEvent(QWheelEvent *event)
{
    if ((m_orientation == Qt::Horizontal) && (event->modifiers() != Qt::ShiftModifier))
    {
        return;
    }

    QPoint numDegrees = event->angleDelta() / 8;

    if (!numDegrees.isNull())
    {
        scrollWithDegrees(numDegrees.y());
    }

    event->accept();
}

void LongScrollBar::scrollWithDegrees(int degrees)
{
    const int sign = (degrees < 0) ? 1 : -1;
    if (m_wheelDegrees.first == sign)
    {
        m_wheelDegrees.second += std::abs(degrees);
    }
    else
    {
        m_wheelDegrees.first = sign;
        m_wheelDegrees.second = std::abs(degrees);
    }

    // qDebug() << "scrollWithDegrees: " << m_wheelDegrees.first*m_wheelDegrees.second;

    if (m_wheelDegrees.second >= 15)
    {
        const ssize_t steps = m_wheelDegrees.second / 15;
        movePosSteps(steps * m_wheelDegrees.first);
        m_wheelDegrees.second = 0;
    }
}

void LongScrollBar::movePosSteps(ssize_t steps)
{
    if (m_pos + steps > m_max || m_pos + steps < 0)
    {
        return;
    }

    changePos(m_pos + (steps * m_posPerStep));
    updateKnob();
    emit posChanged();
}

void LongScrollBar::updateKnob()
{
    if (m_orientation == Qt::Vertical)
    {
        m_knobRect.moveTop(m_pos * m_sizePerPos);
    }
    else
    {
        m_knobRect.moveLeft(m_pos * m_sizePerPos);
    }

    update();
}

void LongScrollBar::changeMax(ssize_t max)
{
    if (max < 0)
    {
        max = 0;
    }

    if (max != m_max)
    {
        if (m_pos > max)
        {
            changePos(max);
        }
        m_max = max;
        // qDebug() << "Max changed:" << m_max;
    }
}

void LongScrollBar::changePos(ssize_t pos)
{
    if (pos < 0)
    {
        pos = 0;
    }
    else if (pos > m_max)
    {
        pos = m_max;
    }

    if (m_pos != pos)
    {
        m_pos = pos;
        // qDebug() << "Pos changed:" << m_pos;
    }
}