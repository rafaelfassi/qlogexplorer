// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QLabel>

class ProgressLabel : public QLabel
{
    Q_OBJECT

public:
    ProgressLabel(QWidget *parent = nullptr);
    void updateProgress();

public slots:
    void setText(const QString &text);
    void setActionText(const QString &text);
    void setProgress(int progress);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *paintEvent) override;

private:
    bool inProgress() const { return (-1 < m_progress) && (m_progress < 100); }
    QString getDisplayText() const;

    QString m_text;
    QString m_actionText;
    int m_progress = -1;
    QPalette m_oriPalette;
};
