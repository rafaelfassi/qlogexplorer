#pragma once

#include <QWidget>

class AbstractModel;
class HeaderView;
class LongScrollBar;
class QPushButton;
class QVBoxLayout;

enum class ColumnsFit
{
    Headers,
    Content,
    Screen
};

class LogViewWidget : public QWidget
{
    Q_OBJECT

public:
    LogViewWidget(QWidget *parent = nullptr);
    ~LogViewWidget();

    void setLogModel(AbstractModel *logModel);

signals:
    void rowSelected(ssize_t row);

public slots:
    void updateView();
    void goToRow(ssize_t row);

protected slots:
    void updateDisplaySize();
    void modelCountChanged();
    void headerChanged();
    void vScrollBarPosChanged();
    void hScrollBarPosChanged();
    void updateRowWidth();
    void expandColumnToContent(ssize_t columnIdx);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    ssize_t getTextWidth(const std::string &text, bool simplified = false);
    QString getElidedText(const std::string &text, ssize_t width, bool simplified = false);

    void fillColumns();
    void adjustColumnsToHeader(std::map<ssize_t, ssize_t> &columnSizesMap);
    void adjustColumnsToContent(std::map<ssize_t, ssize_t> &columnSizesMap);
    void adjustColumnsToScreen(std::map<ssize_t, ssize_t> &columnSizesMap);
    void adjustColumns(ColumnsFit fit);

private:
    AbstractModel *m_logModel = nullptr;
    HeaderView *m_header = nullptr;
    LongScrollBar *m_vScrollBar = nullptr;
    LongScrollBar *m_hScrollBar = nullptr;
    QVBoxLayout *m_vScrollBarLayout = nullptr;
    QVBoxLayout *m_hScrollBarLayout = nullptr;
    QPushButton *m_btnExpandColumns = nullptr;
    QPushButton *m_btnFitColumns = nullptr;
    QTimer *m_updateTimer;
    QFont m_font;
    QFontMetrics m_fm;
    ssize_t m_rowHeight = 0;
    ssize_t m_rowWidth = 0;
    ssize_t m_itemsPerPage = 0;
    QRect m_textAreaRect;
    std::optional<ssize_t> m_currentRow;
};
