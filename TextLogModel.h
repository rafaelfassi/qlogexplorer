#pragma once

#include "AbstractLogModel.h"

class TextLogModel : public AbstractLogModel
{
    Q_OBJECT

public:
    TextLogModel(const std::string &fileName, QObject *parent = 0);

protected:
    void configure(std::istream &is) override;
    bool parseRow(const std::string &rawText, std::vector<std::string> &rowData) const override;
    std::size_t parseChunks(
        std::istream &is,
        std::vector<Chunk> &chunks,
        std::size_t fromPos,
        std::size_t nextRow,
        std::size_t fileSize) override;
    virtual void loadChunkRows(std::istream &is, ChunkRows &chunkRows) const override;
};
