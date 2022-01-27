#include "pch.h"
#include "ProxyModel.h"
#include <execution>

ProxyModel::ProxyModel(AbstractModel *source) : AbstractModel(source), m_source(source)
{
    connect(m_source, &AbstractModel::modelConfigured, this, &AbstractModel::modelConfigured);
}

tp::SInt ProxyModel::getRow(std::uint64_t row, tp::RowData &rowData) const
{
    if (row < m_rowMap.size())
    {
        return m_source->getRow(m_rowMap[row], rowData);
    }
    return -1;
}

tp::Columns &ProxyModel::getColumns()
{
    return m_source->getColumns();
}

const tp::Columns &ProxyModel::getColumns() const
{
    return m_source->getColumns();
}

tp::UInt ProxyModel::columnCount() const
{
    return m_source->columnCount();
}

tp::UInt ProxyModel::rowCount() const
{
    return m_rowMap.size();
}

tp::SInt ProxyModel::getRowNum(tp::SInt row) const
{
    if (row < m_rowMap.size())
    {
        return m_source->getRowNum(m_rowMap[row]);
    }

    return -1;
}

tp::SInt ProxyModel::findSourceRow(tp::SInt srcRow) const
{
    auto it = std::lower_bound(m_rowMap.begin(), m_rowMap.end(), srcRow);
    return (it != m_rowMap.end() && srcRow == *it) ? std::distance(m_rowMap.begin(), it) : -1L;
}

bool ProxyModel::constainsSourceRow(tp::SInt srcRow) const
{
    const auto it = std::lower_bound(m_rowMap.begin(), m_rowMap.end(), srcRow);
    return (it != m_rowMap.end() && srcRow == *it);
}

void ProxyModel::addSourceRow(tp::SInt srcRow)
{
    if (!constainsSourceRow(srcRow))
    {
        m_rowMap.push_back(srcRow);
        std::sort(std::execution::par, m_rowMap.begin(), m_rowMap.end());
        emit countChanged();
    }
}

void ProxyModel::addSourceRows(const tp::SIntList &srcRows)
{
    const tp::UInt oldSize(m_rowMap.size());

    for (const auto srcRow : srcRows)
    {
        if (!constainsSourceRow(srcRow))
        {
            m_rowMap.push_back(srcRow);
        }
    }

    std::sort(std::execution::par, m_rowMap.begin(), m_rowMap.end());

    if (m_rowMap.size() != oldSize)
    {
        emit countChanged();
    }
}

void ProxyModel::removeSourceRow(tp::SInt srcRow)
{
    const auto pos = findSourceRow(srcRow);
    if (pos != -1)
    {
        m_rowMap.erase(m_rowMap.begin() + pos);
        emit countChanged();
    }
}

void ProxyModel::clear()
{
    if (!m_rowMap.empty())
    {
        m_rowMap.clear();
        emit countChanged();
    }
}