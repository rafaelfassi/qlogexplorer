#pragma once

#include "AbstractModel.h"
#include <set>

class ProxyModel : public AbstractModel
{
    Q_OBJECT

public:
    ProxyModel(AbstractModel *source);
    ssize_t getRow(std::uint64_t row, std::vector<std::string> &rowData) const override;
    const std::vector<std::string> &getColumns() const override;
    std::size_t columnCount() const override;
    std::size_t rowCount() const override;
    ssize_t getRowNum(ssize_t row) const override;

    void addRow(std::size_t srcRow);
    void removeRow(std::size_t srcRow);
    void clear();

signals:
    void countChanged();

private:
    AbstractModel *m_source;
    std::set<std::size_t> m_rowMap;
};
