#include "TextLogModel.h"
#include <QDebug>

constexpr size_t g_maxChunksPerParse(500);

TextLogModel::TextLogModel(const std::string &fileName, QObject *parent) : BaseLogModel(fileName, parent)
{
}

TextLogModel::~TextLogModel()
{
    stop();
}

void TextLogModel::configure(std::istream &is)
{
    addColumn(std::string());
}

bool TextLogModel::parseRow(const std::string &rawText, std::vector<std::string> &rowData) const
{
    rowData.emplace_back(rawText);
    return true;
}

std::size_t TextLogModel::parseChunks(
    std::istream &is,
    std::vector<Chunk> &chunks,
    std::size_t fromPos,
    std::size_t nextRow,
    std::size_t fileSize)
{
    std::string buffer;
    buffer.resize(g_chunkSize);

    const size_t totalChunks(std::max<size_t>((fileSize - fromPos) / g_chunkSize, 1));
    chunks.reserve(totalChunks);

    size_t lastPos(0);
    size_t lastLineBreakPos(fromPos);
    size_t nextFirstChunkRow(nextRow);
    size_t currentRowCount(nextRow);

    while (!isEndOfFile(is) && (chunks.size() < g_maxChunksPerParse))
    {
        size_t chunkStartPos = getFilePos(is);
        lastPos = chunkStartPos;
        const size_t readBytes = std::min<size_t>(g_chunkSize, fileSize - lastPos);
        if (readBytes == 0)
        {
            break;
        }

        readFile(is, buffer, readBytes);

        for (size_t i = 0; i < readBytes; ++i)
        {
            ++lastPos;
            if (buffer[i] == '\n')
            {
                ++currentRowCount;
                lastLineBreakPos = lastPos;
            }
        }

        // Is there more characters after the last line break?
        if (lastPos > lastLineBreakPos)
        {
            if (lastPos < fileSize)
            {
                // If it's not the end of the file, move the cursor back to the last line
                // break, so the extra read characters will be include into the next chunk.
                moveFilePos(is, lastLineBreakPos);
                lastPos = lastLineBreakPos;
            }
            else
            {
                // If it's the end of the file, add the extra characters as a new line, in this
                // case the log does not end with a new line.
                ++currentRowCount;
            }
        }

        if (currentRowCount >= nextFirstChunkRow)
        {
            chunks.emplace_back(chunkStartPos, lastPos, nextFirstChunkRow, currentRowCount - 1);
        }
        nextFirstChunkRow = currentRowCount;
    }

    return lastPos;
}

void TextLogModel::loadChunkRows(std::istream &is, ChunkRows &chunkRows) const
{
    moveFilePos(is, chunkRows.getChunk()->getStartPos());

    const auto lastRow = chunkRows.getChunk()->getLastRow();
    auto curentRow = chunkRows.getChunk()->getFistRow();

    chunkRows.reserve(lastRow - curentRow + 1);

    std::string line;
    while ((curentRow <= lastRow) && std::getline(is, line))
    {
        chunkRows.add(curentRow, line);
        ++curentRow;
    }
}
