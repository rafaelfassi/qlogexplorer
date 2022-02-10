// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "LongScrollBar.h"
#include "Style.h"
#include <QPaintEvent>
#include <QPainter>

LongScrollBar::LongScrollBar(Qt::Orientation orientation, QWidget *parent) : QWidget(parent), m_orientation(orientation)
{
    m_knobRect.setLeft(0);
    m_knobRect.setTop(0);
}

LongScrollBar::~LongScrollBar()
{
}

void LongScrollBar::setMax(tp::SInt max)
{
    if (max != m_max)
    {
        const auto pos(m_pos);
        changeMax(max);

        if (m_max > 0)
        {
            updateDisplaySize();
            updateKnob();
        }
        else
        {
            update();
        }

        if (pos != m_pos)
        {
            emit posChanged();
        }
    }
}

tp::SInt LongScrollBar::getMax() const
{
    return m_max;
}

void LongScrollBar::setPos(tp::SInt pos)
{
    if (pos != m_pos)
    {
        changePos(pos);
        updateKnob();
    }
}

tp::SInt LongScrollBar::getPos() const
{
    return m_pos;
}

void LongScrollBar::setPosPerStep(int positions)
{
    m_posPerStep = positions;
}

bool LongScrollBar::isKnobGrabbed()
{
    return m_knobGrabbed.has_value();
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
    const double maxPos(std::max<tp::SInt>(m_max, 1L));

    m_sizePerPos = newHeight / maxPos;
    int knobHeight = std::min(std::max<double>(m_sizePerPos, newHeight / 8.0), newHeight / 2.0);

    m_sizePerPos = (newHeight - knobHeight) / maxPos;

    m_knobRect.setHeight(knobHeight);
    m_knobRect.setWidth(width());
}

void LongScrollBar::updateDisplaySizeH()
{
    const double newWidth(width());
    const double maxPos(std::max<tp::SInt>(m_max, 1L));

    m_sizePerPos = newWidth / maxPos;
    int knobWidth = std::min(std::max<double>(m_sizePerPos, newWidth / 8.0), newWidth / 2.0);

    m_sizePerPos = (newWidth - knobWidth) / maxPos;

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

    devicePainter.fillRect(rect(), Style::getScrollBarColor().bg);
    if (m_max > 0)
    {
        devicePainter.fillRect(m_knobRect, Style::getScrollBarColor().fg);
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
}

void LongScrollBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_knobGrabbed.has_value())
    {
        m_knobGrabbed = std::nullopt;
    }
}

void LongScrollBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_knobGrabbed.has_value())
    {
        tp::SInt newPos(0);
        if (m_orientation == Qt::Vertical)
        {
            const tp::SInt newY =
                std::min(std::max(event->pos().y() - m_knobGrabbed.value(), 0), height() - m_knobRect.height());
            newPos = std::round(newY / m_sizePerPos);
        }
        else
        {
            const tp::SInt newX =
                std::min(std::max(event->pos().x() - m_knobGrabbed.value(), 0), width() - m_knobRect.width());
            newPos = std::round(newX / m_sizePerPos);
        }

        changePos(std::min<tp::SInt>(newPos, m_max));
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

    if (m_wheelDegrees.second >= 15)
    {
        const tp::SInt steps = m_wheelDegrees.second / 15;
        movePosSteps(steps * m_wheelDegrees.first);
        m_wheelDegrees.second = 0;
    }
}

void LongScrollBar::movePosSteps(tp::SInt steps)
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

void LongScrollBar::changeMax(tp::SInt max)
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
    }
}

void LongScrollBar::changePos(tp::SInt pos)
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
    }
}