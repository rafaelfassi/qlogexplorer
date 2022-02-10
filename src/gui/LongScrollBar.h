// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QWidget>

class LongScrollBar : public QWidget
{
    Q_OBJECT

public:
    LongScrollBar(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~LongScrollBar();
    void setMax(tp::SInt max);
    tp::SInt getMax() const;
    void setPos(tp::SInt pos);
    tp::SInt getPos() const;
    void setPosPerStep(int positions);
    bool isKnobGrabbed();
    void wheelEvent(QWheelEvent *event) override;

signals:
    void posChanged();

protected:
    void updateDisplaySize();
    void updateDisplaySizeV();
    void updateDisplaySizeH();

    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void scrollWithDegrees(int degrees);
    void movePosSteps(tp::SInt steps);
    void updateKnob();
    void changeMax(tp::SInt max);
    void changePos(tp::SInt pos);

private:
    Qt::Orientation m_orientation;
    tp::SInt m_max = 0;
    tp::SInt m_pos = 0;
    QRect m_knobRect;
    std::optional<int> m_knobGrabbed;
    double m_sizePerPos = 1.0;
    std::pair<int, int> m_wheelDegrees = {false, 0};
    int m_posPerStep = 1;
};
