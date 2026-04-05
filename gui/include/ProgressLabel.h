// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QLabel>
#include <QPalette>

class QPushButton;

class ProgressLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ProgressLabel(QWidget *parent = nullptr);

    void setText(const QString &text);
    void setActionText(const QString &text);
    void setProgress(int progress);
    void setCanAbort(bool canAbort);
    bool inProgress() const { return m_progress >= 0 && m_progress < 100; }

signals:
    void abortRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *paintEvent) override;

private:
    void updateProgress();
    void updateAbortButton();
    QString getDisplayText() const;

    QString m_text;
    QString m_actionText;
    int m_progress{-1}; // -1 = not in progress
    bool m_canAbort{false};

    QPalette m_oriPalette;
    QPushButton *m_abortButton{nullptr};
};
