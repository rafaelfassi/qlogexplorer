// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "gtest/gtest.h"
#include "InFileStream.h"
#include "model/BaseLogModel.h"

namespace tutl
{

class LogModelSearchResults : public QObject
{
public:
    LogModelSearchResults() {}
    LogModelSearchResults(BaseLogModel &model) { connectModel(&model); }
    LogModelSearchResults(BaseLogModel *model) { connectModel(model); }

    void connectModel(BaseLogModel *model)
    {
        m_model = model;
        connect(model, &BaseLogModel::valueFound, this, &LogModelSearchResults::addSearchResult);
    }

    bool nextRow(tp::RowData &rowData)
    {
        if (m_model.has_value() && m_pos < m_rows.size())
        {
            if ((m_model.value()->getRow(m_rows.at(m_pos), rowData) == m_rows.at(m_pos)))
            {
                ++m_pos;
                return true;
            }
        }
        return false;
    }

    std::size_t count() const { return m_rows.size(); }
    tp::SIntList &getResultRows() { return m_rows; }

public slots:
    void addSearchResult(tp::SharedSIntList rowsPtr)
    {
        for (auto row : *rowsPtr.get())
        {
            m_rows.push_back(row);
        }

        if (m_model.has_value())
        {
            m_model.value()->resultsDigested();
        }
    }

    std::size_t m_pos = 0;
    tp::SIntList m_rows;
    std::optional<BaseLogModel *> m_model;
};

InFileStream::Ptr openTestFile(const std::string &fileName);
void copyResourcesToCurrentDir();

} // namespace tutl
