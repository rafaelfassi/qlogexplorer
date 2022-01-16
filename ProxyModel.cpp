#include "pch.h"
#include "ProxyModel.h"

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

const std::vector<std::string> &ProxyModel::getColumns() const
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

void ProxyModel::addRow(ssize_t srcRow)
{
    m_rowMap.push_back(srcRow);
    std::sort(m_rowMap.begin(), m_rowMap.end());
}

void ProxyModel::addRows(const std::deque<ssize_t> &srcRows)
{
    m_rowMap.insert(m_rowMap.end(), srcRows.begin(), srcRows.end());
    std::sort(m_rowMap.begin(), m_rowMap.end());
}

void ProxyModel::removeRow(ssize_t srcRow)
{
    auto it = std::find(m_rowMap.begin(), m_rowMap.end(), srcRow);
    if (it != m_rowMap.end())
    {
        m_rowMap.erase(it);
    }
}

void ProxyModel::clear()
{
    m_rowMap.clear();
}