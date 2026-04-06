// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "model/TextLogModel.h"

TextLogModel::TextLogModel(FileConf::Ptr conf, QObject *parent) : BaseLogModel(conf, parent)
{
}

TextLogModel::~TextLogModel()
{
    stop();
}

bool TextLogModel::configure(FileConf::Ptr conf, std::istream &is)
{
    if (conf->getRegexPattern().empty() || !conf->hasDefinedColumns())
    {
        if (conf->getColumns().empty())
        {
            conf->addColumn(tp::Column(0));
        }
        m_rx.reset();
    }
    else
    {
        m_rx = RegexBuilder::build(conf->getRegexPattern());
        if (m_rx->hasError())
        {
            LOG_ERR("Invalid regex pattern: '{}': {}", conf->getRegexPattern(), m_rx->getError());
            m_rx.reset();
        }
    }

    return !conf->getColumns().empty();
}

tp::UInt TextLogModel::parseChunks(
    std::istream &is,
    Chunks &chunks,
    tp::UInt fromPos,
    tp::UInt nextRow,
    tp::UInt fileSize)
{
    tp::UInt chunkSize(g_chunkSize);
    std::string buffer;
    buffer.resize(g_chunkSize);

    tp::UInt lastPos(0);
    tp::UInt lastLineBreakPos(fromPos);
    tp::UInt nextFirstChunkRow(nextRow);
    tp::UInt currentRowCount(nextRow);
    const auto chunksPerParse = getChunksPerParse();

    while (!isEndOfFile(is) && (chunks.size() < chunksPerParse))
    {
        tp::UInt chunkStartPos = getFilePos(is);
        lastPos = chunkStartPos;
        const tp::UInt readBytes = std::min<tp::UInt>(chunkSize, fileSize - lastPos);
        if (readBytes == 0)
        {
            break;
        }

        is.read(buffer.data(), readBytes);

        for (tp::UInt i = 0; i < readBytes; ++i)
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

                // If no new row was added in the chunk, the row size is bigger than the chunk.
                if (currentRowCount == nextFirstChunkRow)
                {
                    // Expand chunk size
                    chunkSize *= 2;
                    buffer.resize(chunkSize);
                    continue;
                }
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

void TextLogModel::loadChunkRows(ChunkRows &chunkRows) const
{
    const auto lastRow = chunkRows.getChunk()->getLastRow();
    auto curentRow = chunkRows.getChunk()->getFistRow();
    const auto chunkSize = chunkRows.getChunk()->getSize();
    std::string &buffer = chunkRows.data();
    chunkRows.reserveRows();

    char *begin = buffer.data();
    std::size_t iniPos = 0;
    for (size_t i = 0; i < chunkSize; ++i)
    {
        if (buffer[i] == '\n')
        {
            chunkRows.add(curentRow, std::string_view(begin, i - iniPos));
            ++curentRow;
            iniPos = i + 1;
            begin = buffer.data() + iniPos;
        }
    }

    if (curentRow <= lastRow)
    {
        chunkRows.add(curentRow, std::string_view(begin));
        ++curentRow;
    }
}

bool TextLogModel::parseRow(std::string_view rawText, tp::RowData &rowData) const
{
    // If the file uses "\r\n" as ending line, there will be "\r" at the end of each line
    // because the lines are split by "\n" only.
    if (rawText.back() == '\r')
        rawText.remove_suffix(1);

    if (!m_rx)
    {
        rowData.emplace_back(rawText);
    }
    else
    {
        auto match = m_rx->match(rawText);
        if (match->hasMatch())
        {
            std::string value;

            for (const auto &col : getColumns())
            {
                try
                {
                    if (!col.key.empty())
                    {
                        if (QChar::isDigit(col.key.front()))
                        {
                            value = match->getCaptured(std::stoi(col.key));
                        }
                        else
                        {
                            value = match->getCaptured(col.key);
                        }
                    }
                    else
                    {
                        value.clear();
                    }

                    rowData.push_back(std::move(value));
                }
                catch (const std::exception &e)
                {
                    LOG_ERR("Invalid regex group: {}", col.key);
                    rowData.push_back(std::string());
                }
            }
        }
        else
        {
            rowData.resize(columnCount());
            const auto noMatchCol = getNoMatchColumn();
            if (noMatchCol < rowData.size())
            {
                rowData[noMatchCol] = rawText;
            }
        }
    }

    return true;
}
