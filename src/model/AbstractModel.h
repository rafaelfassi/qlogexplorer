// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#pragma once

#include <QObject>

class AbstractModel : public QObject
{
    Q_OBJECT

public:
    AbstractModel(QObject *parent = 0) : QObject(parent) {}
    virtual tp::SInt getRow(tp::SInt row, tp::RowData &rowData) const = 0;
    virtual tp::Columns &getColumns() = 0;
    virtual const tp::Columns &getColumns() const = 0;
    virtual tp::UInt columnCount() const = 0;
    virtual tp::UInt rowCount() const = 0;
    virtual tp::SInt getRowNum(tp::SInt row) const = 0;

signals:
    void modelConfigured() const;
    void countChanged() const;
};
