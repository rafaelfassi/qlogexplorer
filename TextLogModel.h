#pragma once

#include "AbstractLogModel.h"

class TextLogModel : public AbstractLogModel
{
    Q_OBJECT

public:
    TextLogModel(const std::string& fileName, QObject *parent = 0);

protected:
    bool parseRow(const std::string& rawText, std::vector<std::string>& rowData) const override;
    virtual std::size_t parseChunks(std::size_t fromPos, std::size_t fileSize) override;
    virtual void loadChunkRows(ChunkRows& chunkRows) const override;
};

