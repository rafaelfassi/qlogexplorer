#pragma once

#include <QWidget>

class AbstractModel;
class LongScrollBar;
class QVBoxLayout;

class LogViewWidget : public QWidget
{
    Q_OBJECT

    struct TableColumn
    {
        ssize_t modelIdx;
        std::string name;
        ssize_t width;
        ssize_t maxWidth;
        bool fixedSize = false;
    };

public:
    LogViewWidget(QWidget *parent = nullptr);
    ~LogViewWidget();

    void setLogModel(AbstractModel *logModel);

signals:
    void rowSelected(ssize_t row);

public slots:
    void updateView();
    void updateDisplaySize();
    void modelCountChanged();
    void goToRow(ssize_t row);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    ssize_t getTextWidth(const std::string& text, QString *qStrText = nullptr);
    void fillColumns();

private:
    AbstractModel *m_logModel = nullptr;
    LongScrollBar *m_vScrollBar = nullptr;
    LongScrollBar *m_hScrollBar = nullptr;
    QVBoxLayout *m_vScrollBarLayout = nullptr;
    QVBoxLayout *m_hScrollBarLayout = nullptr;
    QTimer *m_updateTimer;
    QFont m_font;
    QFontMetrics m_fm;
    ssize_t m_rowHeight = 0;
    ssize_t m_itemsPerPage = 0;
    ssize_t m_headerHeight = 0;
    ssize_t m_totalWidthInPage = 0;
    QRect m_textAreaRect;
    std::optional<ssize_t> m_currentRow;
    std::vector<TableColumn> m_columns;
};
