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
    bool parseRow(const std::string &rawText, std::vector<std::string> &rowData) const override;
    std::size_t parseChunks(
        std::istream &is,
        std::vector<Chunk> &chunks,
        std::size_t fromPos,
        std::size_t nextRow,
        std::size_t fileSize) override;
    virtual void loadChunkRows(std::istream &is, ChunkRows &chunkRows) const override;

private:
    QRegularExpression m_rx;
};
