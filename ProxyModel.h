#pragma once

#include "AbstractModel.h"

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

    ssize_t findSourceRow(ssize_t srcRow) const;
    bool constainsSourceRow(ssize_t srcRow) const;
    void addSourceRow(ssize_t srcRow);
    void addSourceRows(const std::deque<ssize_t> &srcRows);
    void removeSourceRow(ssize_t srcRow);
    void clear();

signals:
    void countChanged();

private:
    AbstractModel *m_source;
    std::deque<ssize_t> m_rowMap;
};
