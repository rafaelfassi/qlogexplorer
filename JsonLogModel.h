#pragma once

#include "AbstractLogModel.h"

class JsonLogModel : public AbstractLogModel
{
    Q_OBJECT

public:
    JsonLogModel(const std::string& fileName, QObject *parent = 0);

protected:
    bool parseRow(const std::string& rawText, std::vector<std::string>& rowData) const override;
    virtual std::size_t parseChunks(std::size_t fromPos, std::size_t fileSize) override;
    virtual void loadChunkRows(ChunkRows& chunkRows) const override;
};

