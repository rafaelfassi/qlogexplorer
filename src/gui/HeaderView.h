// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

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
    void setColumns(tp::Columns &columns);

    tp::ColumnsRef m_columns;
    friend class ::HeaderView;
};
} // namespace priv

class HeaderView : public QHeaderView
{
    Q_OBJECT

public:
    HeaderView(QWidget *parent = nullptr);
    ~HeaderView();
    void setColumns(tp::Columns &columns);
    void getVisibleColumns(tp::ColumnsRef &columnsRef, bool orderByPos = false);
    tp::ColumnsRef &getColumns();

signals:
    void columnsChanged();
    void expandToContent(tp::SInt colIdx);
    void expandAllToContent();
    void expandAllToScreen();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void enableBaseSignals();
    void disableBaseSignals();

public slots:
    void updateColumns();

private slots:
    void changedColumnsCount(int, int);
    void movedColumns(int, int, int);
    void resizedColumn(int idx, int oldSize, int size);
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
