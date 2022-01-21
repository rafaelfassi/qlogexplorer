#pragma once

#include <QHeaderView>
#include <QAbstractTableModel>

class QAction;
class HeaderView;

namespace priv
{
class HeaderModel : public QAbstractTableModel
{
    Q_OBJECT
    HeaderModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void addColumn(const std::string &colName);
    void setColumns(const std::vector<std::string> &columns);

    std::vector<std::string> m_columns;
    const QFont *m_font = nullptr;
    QBrush m_textBrush = QColor(Qt::white);
    QBrush m_bgBrush = QColor(Qt::blue);
    friend class ::HeaderView;
};
} // namespace priv

// <VisualPos, ColPos>
using VisibleColPos = std::pair<ssize_t, ssize_t>;
using VisibleColumns = std::vector<VisibleColPos>;

class HeaderView : public QHeaderView
{
    Q_OBJECT

public:
    HeaderView(QWidget *parent = nullptr);
    ~HeaderView();
    void setColumns(const tp::Columns &columns);
    const std::vector<std::string> &getColumns();
    void setFont(const QFont *font);
    void setTextColor(const QColor& color);
    void setBgColor(const QColor& color);
    void getVisibleColumns(VisibleColumns &columns);
    VisibleColPos getVisiblePos(ssize_t colIdx, const VisibleColumns &visibleColumns);

signals:
    void expandToContent(ssize_t colIdx);
    void expandAllToContent();
    void expandAllToScreen();

protected:
    void mousePressEvent(QMouseEvent *e) override;

private slots:
    void columnDoubleClicked(int idx);
    void openContextMenu(QPoint pos, int idx);
    void handleContextMenuAction();

private:
    priv::HeaderModel *m_headerModel;
    QAction *m_hide;
    QAction *m_moveLeft;
    QAction *m_moveRight;
    QAction *m_expandColumn;
    QAction *m_expandAllToContent;
    QAction *m_expandAllToScreen;
    std::optional<int> m_contextColumn;
};
