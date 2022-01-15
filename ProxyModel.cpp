#include "ProxyModel.h"

ProxyModel::ProxyModel(AbstractModel *source) : AbstractModel(source), m_source(source)
{
    connect(m_source, &AbstractModel::modelConfigured, this, &AbstractModel::modelConfigured);
}

ssize_t ProxyModel::getRow(std::uint64_t row, std::vector<std::string> &rowData) const
{
    if (row < m_rowMap.size())
    {
        auto it = m_rowMap.begin();
        std::advance(it, row);
        if (it != m_rowMap.end())
        {
            return m_source->getRow(*it, rowData);
        }
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
        auto it = m_rowMap.begin();
        // TODO: Find a better solution as this does not perform well.
        std::advance(it, row);
        if (it != m_rowMap.end())
        {
            return *it;
        }
    }

    return -1;
}

void ProxyModel::addRow(std::size_t srcRow)
{
    m_rowMap.insert(srcRow);
}

void ProxyModel::removeRow(std::size_t srcRow)
{
    m_rowMap.erase(srcRow);
}

void ProxyModel::clear()
{
    m_rowMap.clear();
}