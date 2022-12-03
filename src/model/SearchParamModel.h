// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QPointer>

class SearchParamProxyModel;

enum ParamModelRoles {
    NotPredefinedParam = Qt::UserRole + 1,
    DisplayAndEditRoles = Qt::UserRole + 2
};

class SearchParamModel : public QAbstractTableModel
{
public:
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
    bool matchRowData(const QString &value, const tp::FilterParam &param, int role) const;
    bool isValidIdx(int idx) const;
    QString getItemPattern(int idx) const;
    int findByItemName(const QString &name);

    QString getParamName(const tp::FilterParam& param, int role) const
    {
        if ((role == Qt::DisplayRole) && !param.name.empty())
            return param.name.c_str();
        else
            return param.searchParam.pattern.c_str();
    }

    SearchParamProxyModel* newProxy(QObject* owner, const std::function<tp::SearchParam(void)>& getParamFunc);
    void applyItem(int idx);

    size_t getAppliedItemOrder(const QModelIndex &modelIndex) const
    {
        const auto it = appliedOrderMap.find(modelIndex.row());
        if (it != appliedOrderMap.end())
            return it->second;
        return 0;
    }

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        return getAppliedItemOrder(left) < getAppliedItemOrder(right);
    }

    bool isReady() const
    {
        return m_ready;
    }

private:
    std::vector<tp::FilterParam> m_data;
    std::vector<QPointer<SearchParamProxyModel>> m_proxes;
    std::unordered_map<int, size_t> appliedOrderMap;
    size_t m_appliedItemsCnt{0};
    bool m_ready{false};
};

class SearchParamProxyModel : public QSortFilterProxyModel
{
    SearchParamProxyModel(SearchParamModel *model, QObject* owner, const std::function<tp::SearchParam(void)>& getParamFunc)
        : QSortFilterProxyModel(owner),
          m_model(model),
          m_getParamFunc(getParamFunc)
    {
        setDynamicSortFilter(false);
        setSourceModel(m_model);
    }
    friend class SearchParamModel;

public:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override
    {
        return m_model->lessThan(left, right);
    }

    QModelIndexList match(
        const QModelIndex &,
        int role,
        const QVariant &value,
        int hits = 1,
        Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const override
    {
        QModelIndexList idxLst;
        const auto srcIdxLst = m_model->match(m_model->index(0, 0), role, value, hits, flags);
        for (const auto&srcIdx : srcIdxLst)
        {
            idxLst.append(mapFromSource(srcIdx));
        }
        return idxLst;
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        if (isValidIdx(index.row()) && (role == Qt::EditRole || role == Qt::DisplayRole))
        {
            const QString valueString = value.toString();
            auto &param = m_model->getRowData(toSrcIdx(index.row()));

            LOG_INF("setData - param name {} pattern {}", param.name, param.searchParam.pattern);

            if (m_model->matchRowData(valueString, param, ParamModelRoles::DisplayAndEditRoles))
                return true;

            LOG_INF("setData - config new param - name {}, srcIdx: {}", valueString.toStdString(), toSrcIdx(index.row()));

            //param.name = utl::toStr(valueString);
            param.searchParam = m_getParamFunc();
            param.searchParam.pattern = utl::toStr(valueString);
            emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
            return true;
        }
        return false;
    }

    bool isValidIdx(int idx) const
    {
        return ((-1 < idx) && (idx < rowCount()));
    }

    int toSrcIdx(int idx)
    {
        if (isValidIdx(idx))
        {
            return mapToSource(index(idx, 0)).row();
        }
        return -1;
    }

    int fromSrcIdx(int srcIdx)
    {
        if (m_model->isValidIdx(srcIdx))
        {
            return mapFromSource(m_model->index(srcIdx, 0)).row();
        }
        return -1;
    }

    std::optional<tp::SearchParam> setCurrentItemIdx(int idx)
    {
        if(isValidIdx(idx))
        {
            m_currentSrcIdx = toSrcIdx(idx);
            return m_model->getSearchParam(m_currentSrcIdx);
        }
        m_currentSrcIdx = -1;
        return std::nullopt;
    }

    std::optional<tp::SearchParam> getCurrentItemParam()
    {
        return m_model->getSearchParam(m_currentSrcIdx);
    }

    void setSearchParam(int idx, tp::SearchParam param)
    {
        if (isValidIdx(idx))
        {
            m_model->setSearchParam(toSrcIdx(idx), param);
        }
    }

    void applyCurrentItem()
    {
        if (m_model->isValidIdx(m_currentSrcIdx))
        {
            m_model->setSearchParam(m_currentSrcIdx, m_getParamFunc());
            m_model->applyItem(m_currentSrcIdx);
        }
    }

    int getCurrentIdx()
    {
        return fromSrcIdx(m_currentSrcIdx);
    }

    bool isReady() const
    {
        return m_model->isReady();
    }

private:
    SearchParamModel *m_model;
    int m_currentSrcIdx{-1};
    std::function<tp::SearchParam(void)> m_getParamFunc;
};
