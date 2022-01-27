#include "pch.h"
#include "TextLogModel.h"

constexpr tp::UInt g_maxChunksPerParse(500);

TextLogModel::TextLogModel(Conf &conf, QObject *parent) : BaseLogModel(conf, parent)
{
}

TextLogModel::~TextLogModel()
{
    stop();
}

bool TextLogModel::configure(Conf &conf, std::istream &is)
{
    if (conf.getRegexPattern().empty())
    {
        if (conf.getColumns().empty())
        {
            conf.addColumn(tp::Column());
        }
        if (m_rx.isValid())
        {
            m_rx.setPattern(QString());
        }
    }
    else
    {
        m_rx.setPattern(conf.getRegexPattern().c_str());
        if (!m_rx.isValid())
        {
            LOG_ERR("Invalid regex pattern: '{}'", conf.getRegexPattern());
        }

        if (conf.getColumns().empty())
        {
            const auto groupsCount = m_rx.captureCount();
            const auto &namedGroups = m_rx.namedCaptureGroups();
            for (auto g = 1; g <= groupsCount; ++g)
            {
                tp::Column cl;
                cl.key = std::to_string(g);
                cl.type = tp::ColumnType::Str;

                if (g < namedGroups.size())
                {
                    cl.name = namedGroups.at(g).toStdString();
                }

                if (cl.name.empty())
                {
                    cl.name = cl.key;
                }

                cl.idx = g - 1;
                cl.pos = cl.idx;
                cl.width = -1;
                conf.addColumn(std::move(cl));
            }
        }
    }

    return !conf.getColumns().empty();
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
            for (const auto &col : getColumns())
            {
                try
                {
                    const auto group = std::stoi(col.key);
                    rowData.push_back(match.captured(group).toStdString());
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
            rowData.emplace_back(rawText);
            rowData.resize(columnCount());
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
