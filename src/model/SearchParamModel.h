// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QAbstractTableModel>

class SearchParamModel : public QAbstractTableModel
{
public:
    struct RowData
    {
        RowData() = default;
        RowData(const QString &_name, const tp::SearchParam &_param) : name(_name), param(_param) {}
        QString name;
        tp::SearchParam param;
    };

    SearchParamModel(QObject *parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QMap<int, QVariant> itemData(const QModelIndex &index) const override;
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    QModelIndexList match(
        const QModelIndex &index,
        int role,
        const QVariant &value,
        int hits = 1,
        Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const override;
    bool moveRows(
        const QModelIndex &sourceParent,
        int sourceRow,
        int count,
        const QModelIndex &destinationParent,
        int destinationChild) override;
    QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;

    void loadParams(const tp::FilterParams &params);
    void updateParams(const tp::FilterParams &params);
    tp::SearchParam getSearchParam(int idx);
    void setSearchParam(int idx, tp::SearchParam param);
    tp::FilterParam &getRowData(int idx);
    bool matchRowData(const QString &value, const tp::FilterParam &param) const;
    bool isValidIdx(int idx) const;
    QString getItemName(int idx) const;
    QString getItemPattern(int idx) const;
    int findByItemName(const QString &name);

private:
    std::vector<tp::FilterParam> m_data;
};
