#pragma once

#include <QWidget>

class AbstractModel;
class HeaderView;
class LongScrollBar;
class QPushButton;
class QVBoxLayout;

enum class ColumnsSize
{
    Headers,
    Content,
    Screen
};

class LogViewWidget : public QWidget
{
    Q_OBJECT

    struct VisualColData
    {
        QString text;
        QRect rect;
        std::optional<std::pair<QRect, QString>> selection;
    };

    struct VisualRowData
    {
        std::vector<VisualColData> columns;
        ssize_t number;
        ssize_t row;
        QRect rect;
        QRect numberRect;
        QRect numberAreaRect;
        bool selected = false;
    };

public:
    LogViewWidget(AbstractModel *model, QWidget *parent = nullptr);
    ~LogViewWidget();

    bool canCopy() const;
    AbstractModel *getModel();
    const std::set<ssize_t> &getMarks();
    void clearMarks();
    bool hasMark(ssize_t row) const;

signals:
    void rowSelected(ssize_t row);

public slots:
    void updateView();
    void goToRow(ssize_t row);
    void markRow(ssize_t row);
    void adjustColumns(ColumnsSize size);
    void copySelected();
    void markSelected();
    void goToPrevRow();
    void goToNextRow();
    void goToPrevPage();
    void goToNextPage();
    void gotToFirstRow();
    void gotToLastRow();
    void goLeft();
    void goRight();
    void goFullLeft();
    void goFullRight();
    void goToPrevMark();
    void goToNextMark();

protected slots:
    void configureColumns();
    void updateDisplaySize();
    void modelCountChanged();
    void headerChanged();
    void vScrollBarPosChanged();
    void hScrollBarPosChanged();
    void stabilizedUpdate();
    void expandColumnToContent(ssize_t columnIdx);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void getVisualRowData(ssize_t row, ssize_t rowOffset, ssize_t hOffset, VisualRowData &vrData);
    void forEachVisualRowInPage(const std::function<bool(VisualRowData &)> &callback);
    ssize_t getMaxRowWidth();

    ssize_t getFirstPageRow() const;
    ssize_t getLastPageRow() const;
    ssize_t getRowByScreenPos(int yPos) const;
    ssize_t getTextWidth(const std::string &text, bool simplified = false);
    QString getElidedText(const std::string &text, ssize_t width, bool simplified = false);

    void getColumnsSizeToHeader(std::map<ssize_t, ssize_t> &columnSizesMap);
    void getColumnsSizeToContent(std::map<ssize_t, ssize_t> &columnSizesMap);
    void getColumnsSizeToScreen(std::map<ssize_t, ssize_t> &columnSizesMap);

    QString getSelectedText(const QString &text, const QRect &textRect, const QRect &selRect, QRect &resultRect);
    int getStrStartPos(const QString &text, int left, int *newLeft = nullptr);
    int getStrEndPos(const QString &text, int right, int *newRight = nullptr);

    QString rowToText(ssize_t row);
    QString getSelectedText(ssize_t row);
    void removeMark(ssize_t row);
    void toggleMark(ssize_t row);
    bool hasPrevMark();
    bool hasNextMark();

private:
    AbstractModel *m_model;
    HeaderView *m_header = nullptr;
    LongScrollBar *m_vScrollBar = nullptr;
    LongScrollBar *m_hScrollBar = nullptr;
    QVBoxLayout *m_vScrollBarLayout = nullptr;
    QVBoxLayout *m_hScrollBarLayout = nullptr;
    QPushButton *m_btnExpandColumns = nullptr;
    QPushButton *m_btnFitColumns = nullptr;
    QTimer *m_stabilizedUpdateTimer = nullptr;
    QAction *m_actGoUp;
    QAction *m_actGoDown;
    QAction *m_actGoPrevPage;
    QAction *m_actGoNextPage;
    QAction *m_actGoFirstRow;
    QAction *m_actGoLastRow;
    QAction *m_actGoLeft;
    QAction *m_actGoRight;
    QAction *m_actGoFullLeft;
    QAction *m_actGoFullRight;
    QAction *m_actCopy;
    QAction *m_actMark;
    QAction *m_actPrevMark;
    QAction *m_actNextMark;
    QFont m_font;
    QFontMetrics m_fm;
    ssize_t m_rowHeight = 0;
    ssize_t m_itemsPerPage = 0;
    QRect m_textAreaRect;
    std::optional<ssize_t> m_currentRow;
    std::optional<std::pair<ssize_t, int>> m_startSelect;
    std::optional<std::pair<ssize_t, int>> m_currentSelec;
    std::set<ssize_t> m_marks;
};
