// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "model/BaseLogModel.h"

class JsonLogModel : public BaseLogModel
{
    Q_OBJECT

public:
    JsonLogModel(FileConf::Ptr conf, QObject *parent = 0);
    ~JsonLogModel();

protected:
    bool configure(FileConf::Ptr conf, std::istream &is) override;
    virtual tp::UInt parseChunks(
        std::istream &is,
        Chunks &chunks,
        tp::UInt fromPos,
        tp::UInt nextRow,
        tp::UInt fileSize) override;
    virtual void loadChunkRows(ChunkRows &chunkRows) const override;
    bool parseRow(std::string_view rawText, tp::RowData &rowData) const override;
};
