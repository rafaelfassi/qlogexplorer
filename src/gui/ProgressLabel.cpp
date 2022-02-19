// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "ProgressLabel.h"
#include "Style.h"
#include <QPainter>

ProgressLabel::ProgressLabel(QWidget *parent) : QLabel(parent)
{
    m_oriPalette = palette();
    auto szPolicy = sizePolicy();
    szPolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    setSizePolicy(szPolicy);
    setMinimumWidth(1);
    setTextInteractionFlags(Qt::TextSelectableByMouse);
    setMargin(Style::getTextPadding());
}

QString ProgressLabel::getDisplayText() const
{
    QString text;

    if (inProgress())
    {
        if (!m_text.isEmpty())
            text.append(QString("%1 - ").arg(m_text));

        if (!m_actionText.isEmpty())
            text.append(QString("%1... ").arg(m_actionText));

        text.append(tr("%1%").arg(m_progress));
    }
    else
    {
        text = m_text;
    }

    return Style::getElidedText(text, width() - 2 * Style::getTextPadding(), Qt::ElideLeft);
}

void ProgressLabel::updateProgress()
{
    if (inProgress())
    {
        int xFact = width() * m_progress / 100;
        QLinearGradient linearGrad(xFact - 1, 0, xFact + 1, 0);
        linearGrad.setColorAt(0, m_oriPalette.color(QPalette::Highlight));
        linearGrad.setColorAt(1, m_oriPalette.color(QPalette::Window));

        QPalette pal = m_oriPalette;
        pal.setBrush(backgroundRole(), QBrush(linearGrad));
        setPalette(pal);
    }
    else
    {
        setPalette(m_oriPalette);
    }

    QLabel::setText(getDisplayText());
}

void ProgressLabel::setText(const QString &text)
{
    m_text = text;
    updateProgress();
}

void ProgressLabel::setActionText(const QString &text)
{
    m_actionText = text;
    updateProgress();
}

void ProgressLabel::setProgress(int progress)
{
    if (m_progress != progress)
    {
        m_progress = progress;
        updateProgress();
    }
}

void ProgressLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::setText(getDisplayText());
    QLabel::resizeEvent(event);
}

void ProgressLabel::paintEvent(QPaintEvent *paintEvent)
{
    if (inProgress())
    {
        QPainter painter(this);
        painter.fillRect(0, 0, width(), height(), palette().brush(backgroundRole()));
    }
    QLabel::paintEvent(paintEvent);
}