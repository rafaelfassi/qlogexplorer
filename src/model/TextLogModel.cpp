// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "TextLogModel.h"

constexpr tp::UInt g_maxChunksPerParse(500);

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
        m_rx.setPattern(QString());
    }
    else
    {
        m_rx.setPattern(conf->getRegexPattern().c_str());
        if (!m_rx.isValid())
        {
            LOG_ERR("Invalid regex pattern: '{}': {}", conf->getRegexPattern(), utl::toStr(m_rx.errorString()));
            m_rx.setPattern(QString());
        }
    }

    return !conf->getColumns().empty();
}

bool TextLogModel::parseRow(const std::string &rawText, tp::RowData &rowData) const
{
    if (m_rx.pattern().isEmpty())
    {
        rowData.emplace_back(rawText);
    }
    else
    {
        QRegularExpressionMatch match = m_rx.match(rawText.c_str());
        if (match.hasMatch())
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
                            value = match.captured(std::stoi(col.key)).toStdString();
                        }
                        else
                        {
                            value = match.captured(col.key.c_str()).toStdString();
                        }
                    }
                    else
                    {
                        value.clear();
                    }

                    rowData.push_back(value);
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

tp::UInt TextLogModel::parseChunks(
    std::istream &is,
    std::vector<Chunk> &chunks,
    tp::UInt fromPos,
    tp::UInt nextRow,
    tp::UInt fileSize)
{
    std::string buffer;
    buffer.resize(g_chunkSize);

    const tp::UInt totalChunks(std::max<tp::UInt>((fileSize - fromPos) / g_chunkSize, 1));
    chunks.reserve(totalChunks);

    tp::UInt lastPos(0);
    tp::UInt lastLineBreakPos(fromPos);
    tp::UInt nextFirstChunkRow(nextRow);
    tp::UInt currentRowCount(nextRow);

    while (!isEndOfFile(is) && (chunks.size() < g_maxChunksPerParse))
    {
        tp::UInt chunkStartPos = getFilePos(is);
        lastPos = chunkStartPos;
        const tp::UInt readBytes = std::min<tp::UInt>(g_chunkSize, fileSize - lastPos);
        if (readBytes == 0)
        {
            break;
        }

        readFile(is, buffer, readBytes);

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
