#include "JsonLogModel.h"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <QDebug>

constexpr size_t g_maxChunksPerParse(50);

void parse(rapidjson::IStreamWrapper &isw)
{
    rapidjson::BaseReaderHandler handler;
    rapidjson::Reader reader;
    reader.Parse<rapidjson::kParseStopWhenDoneFlag>(isw, handler);
}

std::string toString(const rapidjson::Value &json)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);
    return buffer.GetString();
}

JsonLogModel::JsonLogModel(const std::string &fileName, QObject *parent) : AbstractLogModel(fileName, parent)
{
    m_columns.push_back("LogLevel");
    m_columns.push_back("LogMessage");
}

bool JsonLogModel::parseRow(const std::string &rawText, std::vector<std::string> &rowData) const
{
    rapidjson::Document d;
    d.Parse<rapidjson::kParseStopWhenDoneFlag>(rawText.c_str());
    for (const auto &col : m_columns)
    {
        const auto &i = d.FindMember(col.c_str());
        if (i != d.MemberEnd())
        {
            rowData.emplace_back(i->value.GetString());
        }
        else
        {
            rowData.emplace_back(std::string());
        }
    }
    return false;
}

// std::size_t JsonLogModel::parseChunks(std::size_t fromPos, std::size_t fileSize)
// {
//     std::string buffer;
//     buffer.resize(g_chunkSize);

//     const size_t totalChunks(std::max<size_t>(fileSize / g_chunkSize, 1));
//     m_chunks.reserve(totalChunks);

//     size_t nextFirstChunkRow(0);
//     size_t currentRowCount(0);
//     size_t last_pos(0);

//     if (!m_chunks.empty())
//     {
//         currentRowCount = m_chunks.back().getLastRow() + 1;
//         nextFirstChunkRow = currentRowCount;
//     }

//     while (!m_ifs.eof())
//     {
//         size_t chunkStartPos = m_ifs.tellg();
//         last_pos = chunkStartPos;
//         const size_t readBytes = std::min<size_t>(g_chunkSize, fileSize - last_pos);
//         if (readBytes == 0)
//         {
//             break;
//         }

//         std::memset(buffer.data(), 0, buffer.size());
//         m_ifs.read(buffer.data(), readBytes);

//         std::stringstream ss(buffer);
//         rapidjson::IStreamWrapper isw(ss);

//         size_t ini_pos = last_pos;

//         while (!ss.eof())
//         {
//             rapidjson::Document d;
//             d.ParseStream<rapidjson::kParseStopWhenDoneFlag>(isw);
//             const bool hasError(d.HasParseError());

//             bool mustAddRowsToChunk(false);
//             if (!hasError)
//             {
//                 ++currentRowCount;
//                 const size_t ss_pos = ss.tellg();
//                 last_pos = ini_pos + ss_pos;
//                 mustAddRowsToChunk = (g_chunkSize < (last_pos - chunkStartPos));
//             }
//             else
//             {
//                 mustAddRowsToChunk = (currentRowCount > nextFirstChunkRow);
//             }

//             if (mustAddRowsToChunk)
//             {
//                 m_chunks.emplace_back(chunkStartPos, last_pos, nextFirstChunkRow, currentRowCount - 1);
//                 nextFirstChunkRow = currentRowCount;
//                 chunkStartPos = last_pos;
//             }

//             if (hasError)
//             {
//                 if (readBytes == g_chunkSize)
//                 {
//                     m_ifs.seekg(last_pos, std::ios::beg);
//                 }
//                 break;
//             }
//         }
//     }

//     return std::max(last_pos, fileSize);
// }

// std::size_t JsonLogModel::parseChunks(std::size_t fromPos, std::size_t fileSize)
//{
//     std::string buffer;
//     buffer.resize(g_chunkSize);

//    const size_t totalChunks(std::max<size_t>(fileSize / g_chunkSize, 1));
//    m_chunks.reserve(totalChunks);

//    size_t nextFirstChunkRow(0);
//    size_t currentRowCount(0);

//    if (!m_chunks.empty())
//    {
//        currentRowCount = m_chunks.back().getLastRow() + 1;
//        nextFirstChunkRow = currentRowCount;
//    }

//    rapidjson::IStreamWrapper isw(getFileStream());

//    size_t chunkStartPos = getFilePos();
//    size_t last_pos = chunkStartPos;

//    while (!isEndOfFile())
//    {
//        rapidjson::Document d;
//        d.ParseStream<rapidjson::kParseStopWhenDoneFlag>(isw);

//        bool mustAddRowsToChunk(false);
//        if (!d.HasParseError())
//        {
//            ++currentRowCount;
//            last_pos = getFilePos();
//            mustAddRowsToChunk = (g_chunkSize < (last_pos - chunkStartPos));
//        }
//        else if (currentRowCount > nextFirstChunkRow)
//        {
//            mustAddRowsToChunk = (currentRowCount > nextFirstChunkRow);
//        }

//        if (mustAddRowsToChunk)
//        {
//            m_chunks.emplace_back(chunkStartPos, last_pos, nextFirstChunkRow, currentRowCount - 1);
//            nextFirstChunkRow = currentRowCount;
//            chunkStartPos = last_pos;
//        }
//    }

//    return std::max(last_pos, fileSize);
//}

std::size_t JsonLogModel::parseChunks(
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

    size_t nextFirstChunkRow(nextRow);
    size_t currentRowCount(nextRow);

    rapidjson::Reader reader;
    rapidjson::BaseReaderHandler handler;
    rapidjson::IStreamWrapper isw(is);

    size_t chunkStartPos = getFilePos(is);
    size_t last_pos = chunkStartPos;

    while (!isEndOfFile(is) && (chunks.size() < g_maxChunksPerParse))
    {
        bool mustAddRowsToChunk(false);
        const auto res = reader.Parse<rapidjson::kParseStopWhenDoneFlag>(isw, handler);
        if (!res.IsError())
        {
            ++currentRowCount;
            last_pos = getFilePos(is);
            mustAddRowsToChunk = (g_chunkSize < (last_pos - chunkStartPos));
        }
        else if (currentRowCount > nextFirstChunkRow)
        {
            mustAddRowsToChunk = true;
        }
        else
        {
            qCritical() << "Error parsing json file at pos" << res.Offset();
            return std::max(last_pos, fileSize);
        }

        if (mustAddRowsToChunk)
        {
            chunks.emplace_back(chunkStartPos, last_pos, nextFirstChunkRow, currentRowCount - 1);
            nextFirstChunkRow = currentRowCount;
            chunkStartPos = last_pos;
        }
    }

    if (isEndOfFile(is))
    {
        return std::max(last_pos, fileSize);
    }
    else
    {
        return last_pos;
    }
}

void JsonLogModel::loadChunkRows(std::istream &is, ChunkRows &chunkRows) const
{
    moveFilePos(is, chunkRows.getChunk()->getStartPos());

    const auto lastRow = chunkRows.getChunk()->getLastRow();
    auto curentRow = chunkRows.getChunk()->getFistRow();

    chunkRows.reserve(lastRow - curentRow + 1);

    rapidjson::IStreamWrapper isw(is);

    while (curentRow <= lastRow)
    {
        rapidjson::Document d;
        d.ParseStream<rapidjson::kParseStopWhenDoneFlag>(isw);
        if (!d.HasParseError())
        {
            chunkRows.add(curentRow, toString(d));
            ++curentRow;
        }
    }
}
