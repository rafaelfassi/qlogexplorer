#pragma once

#include <QObject>

class AbstractModel : public QObject
{
    Q_OBJECT

public:
    AbstractModel(QObject *parent = 0) : QObject(parent) {}
    virtual ssize_t getRow(std::uint64_t row, std::vector<std::string> &rowData) const = 0;
    virtual const tp::Columns &getColumns() const = 0;
    virtual std::size_t columnCount() const = 0;
    virtual std::size_t rowCount() const = 0;
    virtual ssize_t getRowNum(ssize_t row) const = 0;

signals:
    void modelConfigured() const;
    void countChanged() const;
};

