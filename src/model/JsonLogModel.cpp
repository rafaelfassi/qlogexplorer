// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#include "pch.h"
#include "JsonLogModel.h"

constexpr tp::UInt g_maxChunksPerParse(50);

std::string toString(const rapidjson::Value &json)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);
    return buffer.GetString();
}

JsonLogModel::JsonLogModel(Conf &conf, QObject *parent) : BaseLogModel(conf, parent)
{
}

JsonLogModel::~JsonLogModel()
{
    stop();
}

bool JsonLogModel::configure(Conf &conf, std::istream &is)
{
    tp::SInt idx(0);

    if (conf.getColumns().empty())
    {
        rapidjson::IStreamWrapper isw(is);
        rapidjson::Document d;
        d.ParseStream<rapidjson::kParseStopWhenDoneFlag>(isw);
        if (!d.HasParseError())
        {
            for (auto i = d.MemberBegin(); i != d.MemberEnd(); ++i)
            {
                tp::Column cl;
                cl.key = i->name.GetString();
                cl.name = cl.key;
                switch (i->value.GetType())
                {
                case rapidjson::kFalseType:
                case rapidjson::kTrueType:
                    cl.type = tp::ColumnType::Bool;
                    break;
                case rapidjson::kStringType:
                    cl.type = tp::ColumnType::Str;
                    break;
                case rapidjson::kNumberType:
                    if (i->value.IsUint64())
                        cl.type = tp::ColumnType::UInt;
                    if (i->value.IsInt64())
                        cl.type = tp::ColumnType::Int;
                    else
                        cl.type = tp::ColumnType::Float;
                    break;
                default:
                    break;
                }

                cl.idx = idx++;
                cl.pos = cl.idx;
                cl.width = -1;
                conf.addColumn(std::move(cl));
            }
        }
    }
    else
    {
        for (auto& col : conf.getColumns())
        {
            col.idx = idx++;
        }
    }

    return !conf.getColumns().empty();
}

bool JsonLogModel::parseRow(const std::string &rawText, tp::RowData &rowData) const
{
    rapidjson::Document d;
    d.Parse<rapidjson::kParseStopWhenDoneFlag>(rawText.c_str());
    for (const auto &col : getColumns())
    {
        std::string colText;

        const auto &i = d.FindMember(col.key.c_str());
        if (i != d.MemberEnd())
        {
            switch (i->value.GetType())
            {
            case rapidjson::kFalseType:
                colText = "TRUE";
                break;
            case rapidjson::kTrueType:
                colText = "FALSE";
                break;
            case rapidjson::kStringType:
                colText = i->value.GetString();
                break;
            case rapidjson::kNumberType:
                if (i->value.IsUint64())
                    colText = std::to_string(i->value.GetUint64());
                if (i->value.IsInt64())
                    colText = std::to_string(i->value.GetInt64());
                else
                    colText = std::to_string(i->value.GetDouble());
                break;
            default:
                break;
            }
        }
        rowData.emplace_back(std::move(colText));
    }
    return true;
}

tp::UInt JsonLogModel::parseChunks(
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

    tp::UInt nextFirstChunkRow(nextRow);
    tp::UInt currentRowCount(nextRow);

    rapidjson::Reader reader;
    rapidjson::BaseReaderHandler handler;
    rapidjson::IStreamWrapper isw(is);

    tp::UInt chunkStartPos = getFilePos(is);
    tp::UInt last_pos = chunkStartPos;

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
            LOG_ERR("Error parsing json file at pos {}", res.Offset());
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
