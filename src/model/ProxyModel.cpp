#include "pch.h"
#include "ProxyModel.h"
#include <execution>

ProxyModel::ProxyModel(AbstractModel *source) : AbstractModel(source), m_source(source)
{
    connect(m_source, &AbstractModel::modelConfigured, this, &AbstractModel::modelConfigured);
}

ssize_t ProxyModel::getRow(std::uint64_t row, std::vector<std::string> &rowData) const
{
    if (row < m_rowMap.size())
    {
        return m_source->getRow(m_rowMap[row], rowData);
    }
    return -1;
}

const tp::Columns &ProxyModel::getColumns() const
{
    return m_source->getColumns();
}

std::size_t ProxyModel::columnCount() const
{
    return m_source->columnCount();
}

std::size_t ProxyModel::rowCount() const
{
    return m_rowMap.size();
}

ssize_t ProxyModel::getRowNum(ssize_t row) const
{
    if (row < m_rowMap.size())
    {
        return m_source->getRowNum(m_rowMap[row]);
    }

    return -1;
}

ssize_t ProxyModel::findSourceRow(ssize_t srcRow) const
{
    auto it = std::lower_bound(m_rowMap.begin(), m_rowMap.end(), srcRow);
    return (it != m_rowMap.end() && srcRow == *it) ? std::distance(m_rowMap.begin(), it) : -1L;
}

bool ProxyModel::constainsSourceRow(ssize_t srcRow) const
{
    const auto it = std::lower_bound(m_rowMap.begin(), m_rowMap.end(), srcRow);
    return (it != m_rowMap.end() && srcRow == *it);
}

void ProxyModel::addSourceRow(ssize_t srcRow)
{
    if (!constainsSourceRow(srcRow))
    {
        m_rowMap.push_back(srcRow);
        std::sort(std::execution::par, m_rowMap.begin(), m_rowMap.end());
        emit countChanged();
    }
}

void ProxyModel::addSourceRows(const std::deque<ssize_t> &srcRows)
{
    const size_t oldSize(m_rowMap.size());

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

void ProxyModel::removeSourceRow(ssize_t srcRow)
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