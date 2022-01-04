#pragma once

#include "AbstractLogModel.h"

class JsonLogModel : public AbstractLogModel
{
    Q_OBJECT

public:
    JsonLogModel(const std::string &fileName, QObject *parent = 0);
    ~JsonLogModel();

protected:
    void configure(std::istream &is) override;
    bool parseRow(const std::string &rawText, std::vector<std::string> &rowData) const override;
    virtual std::size_t parseChunks(
        std::istream &is,
        std::vector<Chunk> &chunks,
        std::size_t fromPos,
        std::size_t nextRow,
        std::size_t fileSize) override;
    virtual void loadChunkRows(std::istream &is, ChunkRows &chunkRows) const override;
};
