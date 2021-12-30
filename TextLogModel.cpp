#include "TextLogModel.h"

TextLogModel::TextLogModel(const std::string &fileName, QObject *parent) : AbstractLogModel(fileName, parent)
{
    m_columns.push_back("");
}

bool TextLogModel::parseRow(const std::string &rawText, std::vector<std::string> &rowData) const
{
    rowData.emplace_back(rawText);
    return true;
}

std::size_t TextLogModel::parseChunks(std::size_t fromPos, std::size_t fileSize)
{
    std::string buffer;
    buffer.resize(g_chunkSize);

    const size_t totalChunks(std::max<size_t>(fileSize / g_chunkSize, 1));
    m_chunks.reserve(totalChunks);

    size_t lastPos(0);
    size_t lastLineBreakPos(fromPos);
    size_t nextFirstChunkRow(0);
    size_t currentRowCount(0);

    if (!m_chunks.empty())
    {
        currentRowCount = m_chunks.back().getLastRow() + 1;
        nextFirstChunkRow = currentRowCount;
    }

    while (!isEndOfFile())
    {
        size_t chunkStartPos = getFilePos();
        lastPos = chunkStartPos;
        const size_t readBytes = std::min<size_t>(g_chunkSize, fileSize - lastPos);
        if (readBytes == 0)
        {
            break;
        }

        readFile(buffer, readBytes);

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
                moveFilePos(lastLineBreakPos);
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
            m_chunks.emplace_back(chunkStartPos, lastPos, nextFirstChunkRow, currentRowCount - 1);
        }
        nextFirstChunkRow = currentRowCount;
    }

    return std::max(lastPos, fileSize);
}

void TextLogModel::loadChunkRows(ChunkRows &chunkRows) const
{
    moveFilePos(chunkRows.getChunk()->getStartPos());

    const auto lastRow = chunkRows.getChunk()->getLastRow();
    auto curentRow = chunkRows.getChunk()->getFistRow();

    std::string line;
    while ((curentRow <= lastRow) && std::getline(getFileStream(), line))
    {
        chunkRows.add(curentRow, line);
        ++curentRow;
    }
}