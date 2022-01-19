#pragma once

#include <QWidget>

class LongScrollBar : public QWidget
{
    Q_OBJECT

public:
    LongScrollBar(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~LongScrollBar();
    void setMax(ssize_t max);
    ssize_t getMax() const;
    void setPos(ssize_t pos);
    ssize_t getPos() const;
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
    void movePosSteps(ssize_t steps);
    void updateKnob();
    void changeMax(ssize_t max);
    void changePos(ssize_t pos);

private:
    Qt::Orientation m_orientation;
    ssize_t m_max = 0;
    ssize_t m_pos = 0;
    QRect m_knobRect;
    std::optional<int> m_knobGrabbed;
    double m_sizePerPos = 1.0;
    std::pair<int, int> m_wheelDegrees = {false, 0};
    int m_posPerStep = 1;
};
