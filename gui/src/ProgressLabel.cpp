// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "ProgressLabel.h"
#include "Style.h"
#include <QPainter>
#include <QPushButton>

ProgressLabel::ProgressLabel(QWidget *parent) : QLabel(parent)
{
    m_oriPalette = palette();

    auto szPolicy = sizePolicy();
    szPolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    setSizePolicy(szPolicy);
    setMinimumWidth(1);
    setTextInteractionFlags(Qt::TextSelectableByMouse);
    setMargin(Style::getTextPadding());

    m_abortButton = new QPushButton(Style::getIcon("cancel_icon.png"), "", this);
    m_abortButton->setToolTip(tr("Cancel"));
    m_abortButton->setFlat(true);
    m_abortButton->setVisible(false);
    m_abortButton->setFocusPolicy(Qt::NoFocus);

    connect(m_abortButton, &QPushButton::clicked, this, &ProgressLabel::abortRequested);
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

    // Reserve horizontal space for the abort button when it is visible.
    const int reservedWidth = (m_abortButton && m_abortButton->isVisible()) ? m_abortButton->width() : 0;

    return Style::getElidedText(text, width() - (2 * Style::getTextPadding()) - reservedWidth, Qt::ElideLeft);
}

void ProgressLabel::updateAbortButton()
{
    const bool show = m_canAbort && inProgress();
    m_abortButton->setVisible(show);

    if (show)
    {
        // Pin the button to the right edge, vertically centred.
        int btnSz = std::max(m_abortButton->sizeHint().height(), m_abortButton->sizeHint().width());
        btnSz = std::min(height(), btnSz);
        m_abortButton->setGeometry(width() - btnSz - 1, (height() - btnSz) / 2, btnSz, btnSz);
    }
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

    updateAbortButton();
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

void ProgressLabel::setCanAbort(bool canAbort)
{
    if (m_canAbort != canAbort)
    {
        m_canAbort = canAbort;
        if (inProgress())
        {
            updateProgress();
        }
    }
}

void ProgressLabel::resizeEvent(QResizeEvent *event)
{
    updateAbortButton();
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
