// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#pragma once

#include <QWidget>
#include "Highlighter.h"

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

struct TextCan
{
    TextCan() = default;
    TextCan(const QString &_text) : text(_text) {}
    TextCan(const QRect &_rect, const QString &_text = {}) : rect(_rect), text(_text) {}
    QRect rect;
    QString text;
};

struct SectionColor
{
    SectionColor() = default;
    SectionColor(const QColor &_fg, const QColor &_bg) : fg(_fg), bg(_bg) {}
    QColor fg;
    QColor bg;
};

struct TextSelection
{
    TextSelection() = default;
    TextSelection(const TextCan &_can, const SectionColor &_color) : can(_can), color(_color) {}
    TextCan can;
    SectionColor color;
};

class LogViewWidget : public QWidget
{
    Q_OBJECT

    struct VisualColData
    {
        TextCan can;
        std::optional<TextSelection> selection;
        std::vector<TextSelection> markedTexts;
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
        const Highlighter *highlighter = nullptr;
    };

public:
    LogViewWidget(AbstractModel *model, QWidget *parent = nullptr);
    ~LogViewWidget();

    bool canCopy() const;
    AbstractModel *getModel();
    const std::set<tp::SInt> &getBookmarks();
    void clearBookmarks();
    bool hasBookmark(tp::SInt row) const;
    void configure(Conf *conf);

signals:
    void rowSelected(tp::SInt row);

public slots:
    void updateView();
    void resetColumns();
    void goToRow(tp::SInt row);
    void addBookmark(tp::SInt row);
    void adjustColumns(ColumnsFit fit);
    void copySelected();
    void bookmarkSelected();
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
    void goToPrevBookmark();
    void goToNextBookmark();
    void addTextMark(const QString &text, const SectionColor &selColor);
    void removeTextMarks(const SectionColor &selColor);

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
    QString getElidedText(const QString &text, tp::SInt width, bool simplified = false);

    void getColumnsSizeToHeader(tp::ColumnsRef &columnsRef, bool discardConfig = false);
    void getColumnsSizeToContent(tp::ColumnsRef &columnsRef);
    void getColumnsSizeToScreen(tp::ColumnsRef &columnsRef);

    qreal getCharSize();
    qreal getCharMarging();
    std::vector<TextSelection> findMarkedText(const TextCan &can);
    TextCan makeSelCanFromStrPos(const TextCan &can, int fromPos, int len);
    TextCan makeSelCanFromSelRect(const TextCan &can, const QRect &selRect);
    int getStrWidthUntilPos(int pos, int maxWidth = std::numeric_limits<int>::max());
    int getStrStartPos(int left, int *newLeft = nullptr);
    int getStrEndPos(int right, int *newRight = nullptr, int maxSize = std::numeric_limits<int>::max());

    QString rowToText(tp::SInt row);
    QString getTextSelection(tp::SInt row);
    void removeBookmark(tp::SInt row);
    void toggleBookmark(tp::SInt row);
    bool hasPrevBookmark();
    bool hasNextBookmark();
    bool hasTextSelected();

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
    QAction *m_actBookmark;
    QAction *m_actPrevBookmark;
    QAction *m_actNextBookmark;
    QFont m_font;
    QFontMetrics m_fm;
    tp::SInt m_rowHeight = 0;
    tp::SInt m_itemsPerPage = 0;
    QRect m_textAreaRect;
    std::optional<tp::SInt> m_selectedRow;
    std::optional<QString> m_selectedText;
    std::optional<std::pair<tp::SInt, int>> m_selectStart;
    std::optional<std::pair<tp::SInt, int>> m_selectEnd;
    std::set<tp::SInt> m_bookMarks;
    std::vector<Highlighter> m_highlightersRows;
    std::vector<TextSelection> m_markedTexts;
    std::vector<SectionColor> m_availableMarks;
};
