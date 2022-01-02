#pragma once

#include <QWidget>

class AbstractLogModel;
class LongScrollBar;

class LogViewWidget : public QWidget
{
    Q_OBJECT

public:
    LogViewWidget(QWidget *parent = nullptr);
    ~LogViewWidget();

    void setLogModel(AbstractLogModel *logModel);

public slots:
    void updateDisplaySize();
    void modelCountChanged();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    AbstractLogModel *m_logModel = nullptr;
    LongScrollBar *m_vScrollBar = nullptr;
    LongScrollBar *m_hScrollBar = nullptr;
    QFont m_font;
    QFontMetrics m_fm;
    ssize_t m_rowHeight = 0;
    ssize_t m_itemsPerPage = 0;
    int m_biggerTextWidthInPage = 0;
    QRect m_textAreaRect;
    std::optional<ssize_t> m_currentRow;
};
