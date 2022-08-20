#include "pch.h"
#include "SearchParamModel.h"

SearchParamModel::SearchParamModel(QObject *parent) : QAbstractTableModel(parent)
{
}

Qt::ItemFlags SearchParamModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index) | Qt::ItemIsDropEnabled;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QVariant SearchParamModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QAbstractTableModel::headerData(section, orientation, role);
}

int SearchParamModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_data.size();
}

int SearchParamModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant SearchParamModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_data.size())
        return QVariant();
    if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
    {
        const auto &param = m_data.at(index.row());
        return getParamName(param, role);
    }
    return QVariant();
}

bool SearchParamModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    LOG_ERR("setData must be implemented in the proxy model");
    return false;
}

QMap<int, QVariant> SearchParamModel::itemData(const QModelIndex &index) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return QMap<int, QVariant>{};

    auto &param = m_data.at(index.row());
    return QMap<int, QVariant>{
        {std::make_pair<int>(Qt::DisplayRole, getParamName(param, Qt::DisplayRole)),
         std::make_pair<int>(Qt::EditRole, getParamName(param, Qt::EditRole))}};
}

bool SearchParamModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    if (roles.isEmpty())
        return false;
    if (std::any_of(
            roles.keyBegin(),
            roles.keyEnd(),
            [](int role) -> bool { return role != Qt::DisplayRole && role != Qt::EditRole; }))
    {
        return false;
    }

    bool ok(false);
    QMapIterator<int, QVariant> r(roles);
    while (r.hasNext())
    {
        r.next();
        ok = setData(index, r.value(), r.key());
    }

    return ok;
}

bool SearchParamModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0 || row > rowCount(parent))
        return false;

    LOG_INF("insertRows - row: {}, count: {}", row, count);
    beginInsertRows(QModelIndex(), row, row + count - 1);
    m_data.insert(m_data.begin() + row, count, tp::FilterParam());
    endInsertRows();
    return true;
}

bool SearchParamModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
        return false;

    LOG_INF("removeRows - row: {}, count: {}", row, count);
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    const auto it = m_data.begin() + row;
    m_data.erase(it, it + count);
    endRemoveRows();
    return true;
}

QModelIndexList SearchParamModel::match(
    const QModelIndex &index,
    int role,
    const QVariant &value,
    int hits,
    Qt::MatchFlags flags) const
{
    QModelIndexList indexLst;
    if (index.isValid())
    {
        const QString valueString = value.toString();
        for (int row = index.row(); row < rowCount(); ++row)
        {
            const auto &param = m_data.at(row);
            if (matchRowData(valueString, param, role))
            {
                indexLst.append(createIndex(row, 0));
                return indexLst;
            }
        }
    }
    return indexLst;
}

bool SearchParamModel::moveRows(
    const QModelIndex &sourceParent,
    int sourceRow,
    int count,
    const QModelIndex &destinationParent,
    int destinationChild)
{
    LOG_INF("moveRows");
    if (sourceRow < 0 || sourceRow + count - 1 >= rowCount(sourceParent) || destinationChild < 0 ||
        destinationChild > rowCount(destinationParent) || sourceRow == destinationChild || count <= 0 ||
        sourceParent.isValid() || destinationParent.isValid())
    {
        return false;
    }

    if (!beginMoveRows(QModelIndex(), sourceRow, sourceRow + count - 1, QModelIndex(), destinationChild))
        return false;

    const auto pivot = m_data.begin() + sourceRow;
    std::rotate(m_data.begin(), pivot, pivot + count);
    endMoveRows();
    return false;
}

QModelIndex SearchParamModel::sibling(int row, int column, const QModelIndex &idx) const
{
    if (!idx.isValid() || column != 0 || row >= m_data.size() || row < 0)
        return QModelIndex();
    return createIndex(row, 0);
}

void SearchParamModel::loadParams(const tp::FilterParams &params)
{
    LOG_INF("loadParams - count: {}", params.size());
    if (params.empty())
    {
        return;
    }
    const auto fromRow = rowCount();
    const auto toRow = fromRow + params.size() - 1;
    LOG_INF("loadParams - inserting fromRow: {}, toRow: {}", fromRow, toRow);
    beginInsertRows(QModelIndex(), fromRow, toRow);
    for (const auto &param : params)
    {
        m_data.emplace_back(param);
    }
    endInsertRows();

    m_ready = true;
}

void SearchParamModel::updateParams(const tp::FilterParams &params)
{
    LOG_INF("Updating parameters...");
    for (const auto &param : params)
    {
        const auto idx = findByItemName(param.name.c_str());
        if (idx == -1)
        {
            LOG_INF("updateParams - insert - name {}, pattern {}", param.name, param.searchParam.pattern);
            beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
            m_data.emplace_back(param);
            endInsertRows();
        }
        else
        {
            if (m_data.at(idx).searchParam != param.searchParam)
            {
                m_data.at(idx).searchParam = param.searchParam;
            }
        }
    }
}

tp::SearchParam SearchParamModel::getSearchParam(int idx)
{
    if (isValidIdx(idx))
        return m_data.at(idx).searchParam;
    return tp::SearchParam();
}

void SearchParamModel::setSearchParam(int idx, tp::SearchParam param)
{
    if (isValidIdx(idx))
        m_data.at(idx).searchParam = param;
}

tp::FilterParam &SearchParamModel::getRowData(int idx)
{
    return m_data.at(idx);
}

bool SearchParamModel::matchRowData(const QString &value, const tp::FilterParam &param, int role) const
{
    const auto &s(utl::toStr(value));
    switch (role)
    {
        case Qt::DisplayRole:
            return (!param.name.empty() && param.name == s);
        case Qt::EditRole:
            return (param.searchParam.pattern == s);
        case ParamModelRoles::NotPredefinedParam:
            return (param.name.empty() && param.searchParam.pattern == s);
        case ParamModelRoles::DisplayAndEditRoles:
            return ((!param.name.empty() && param.name == s) || param.searchParam.pattern == s);
        default:
            LOG_ERR("Invalid role {}", role);
            break;
    }
    return false;
}

bool SearchParamModel::isValidIdx(int idx) const
{
    return ((-1 < idx) && (idx < rowCount()));
}

QString SearchParamModel::getItemPattern(int idx) const
{
    if (isValidIdx(idx))
        return data(index(idx, 0), Qt::EditRole).toString();
    return QString();
}

int SearchParamModel::findByItemName(const QString &name)
{
    if (!name.isEmpty())
    {
        for (int row = 0; row < rowCount(); ++row)
        {
            if (m_data.at(row).name.c_str() == name)
                return row;
        }
    }
    return -1;
}

SearchParamProxyModel *SearchParamModel::newProxy(
    QObject *owner,
    const std::function<tp::SearchParam(void)> &getParamFunc)
{
    auto proxy = new SearchParamProxyModel(this, owner, getParamFunc);
    m_proxes.emplace_back(proxy);
    proxy->sort(0, Qt::DescendingOrder);
    return proxy;
}

void SearchParamModel::applyItem(int idx)
{
    appliedOrderMap[idx] = ++m_appliedItemsCnt;
    for (auto &proxy : m_proxes)
    {
        if (proxy)
        {
            proxy->sort(0, Qt::DescendingOrder);
        }
    }
}

// **************** SearchParamProxyModel *************************************************************
