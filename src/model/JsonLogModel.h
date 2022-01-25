#pragma once

#include "BaseLogModel.h"

class JsonLogModel : public BaseLogModel
{
    Q_OBJECT

public:
    JsonLogModel(Conf &conf, QObject *parent = 0);
    ~JsonLogModel();

protected:
    bool configure(Conf &conf, std::istream &is) override;
    bool parseRow(const std::string &rawText, std::vector<std::string> &rowData) const override;
    virtual tp::UInt parseChunks(
        std::istream &is,
        std::vector<Chunk> &chunks,
        tp::UInt fromPos,
        tp::UInt nextRow,
        tp::UInt fileSize) override;
    virtual void loadChunkRows(std::istream &is, ChunkRows &chunkRows) const override;
};
