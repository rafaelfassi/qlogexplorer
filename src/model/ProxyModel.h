// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "AbstractModel.h"

class ProxyModel : public AbstractModel
{
    Q_OBJECT

public:
    ProxyModel(AbstractModel *source);
    tp::SInt getRow(tp::SInt row, tp::RowData &rowData) const override;
    tp::Columns &getColumns() override;
    const tp::Columns &getColumns() const override;
    tp::UInt columnCount() const override;
    tp::UInt rowCount() const override;
    tp::SInt getRowNum(tp::SInt row) const override;

    tp::SInt findSourceRow(tp::SInt srcRow) const;
    bool constainsSourceRow(tp::SInt srcRow) const;
    void addSourceRow(tp::SInt srcRow);
    void addSourceRows(const tp::SIntList &srcRows);
    void removeSourceRow(tp::SInt srcRow);
    void clear();

signals:
    void countChanged();

private:
    AbstractModel *m_source;
    tp::SIntList m_rowMap;
};
