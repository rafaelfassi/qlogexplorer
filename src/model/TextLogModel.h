// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "BaseLogModel.h"

class TextLogModel : public BaseLogModel
{
    Q_OBJECT

public:
    TextLogModel(Conf &conf, QObject *parent = 0);
    ~TextLogModel();

protected:
    bool configure(Conf &conf, std::istream &is) override;
    bool parseRow(const std::string &rawText, tp::RowData &rowData) const override;
    tp::UInt parseChunks(
        std::istream &is,
        std::vector<Chunk> &chunks,
        tp::UInt fromPos,
        tp::UInt nextRow,
        tp::UInt fileSize) override;
    virtual void loadChunkRows(std::istream &is, ChunkRows &chunkRows) const override;

private:
    QRegularExpression m_rx;
};
