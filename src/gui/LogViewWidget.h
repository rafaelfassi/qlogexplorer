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
        tp::SInt number;
        tp::SInt row;
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
    const std::set<tp::SInt> &getMarks();
    void clearMarks();
    bool hasMark(tp::SInt row) const;

signals:
    void rowSelected(tp::SInt row);

public slots:
    void updateView();
    void resetColumns();
    void goToRow(tp::SInt row);
    void markRow(tp::SInt row);
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
    void expandColumnToContent(tp::SInt columnIdx);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void getVisualRowData(tp::SInt row, tp::SInt rowOffset, tp::SInt hOffset, VisualRowData &vrData);
    void forEachVisualRowInPage(const std::function<bool(VisualRowData &)> &callback);
    tp::SInt getMaxRowWidth();

    tp::SInt getFirstPageRow() const;
    tp::SInt getLastPageRow() const;
    tp::SInt getRowByScreenPos(int yPos) const;
    tp::SInt getTextWidth(const std::string &text, bool simplified = false);
    QString getElidedText(const std::string &text, tp::SInt width, bool simplified = false);

    void getColumnsSizeToHeader(tp::ColumnsRef &columnsRef);
    void getColumnsSizeToContent(tp::ColumnsRef &columnsRef);
    void getColumnsSizeToScreen(tp::ColumnsRef &columnsRef);

    QString getSelectedText(const QString &text, const QRect &textRect, const QRect &selRect, QRect &resultRect);
    int getStrStartPos(const QString &text, int left, int *newLeft = nullptr);
    int getStrEndPos(const QString &text, int right, int *newRight = nullptr);

    QString rowToText(tp::SInt row);
    QString getSelectedText(tp::SInt row);
    void removeMark(tp::SInt row);
    void toggleMark(tp::SInt row);
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
    tp::SInt m_rowHeight = 0;
    tp::SInt m_itemsPerPage = 0;
    QRect m_textAreaRect;
    std::optional<tp::SInt> m_currentRow;
    std::optional<std::pair<tp::SInt, int>> m_startSelect;
    std::optional<std::pair<tp::SInt, int>> m_currentSelec;
    std::set<tp::SInt> m_marks;
};
