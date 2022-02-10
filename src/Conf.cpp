// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "Conf.h"

Conf::Conf(tp::FileType fileType) : m_fileType(fileType)
{
}

Conf::Conf(const std::string &confFileName)
{
    loadFConf(confFileName);
}

bool Conf::loadFConf(const std::string &confFileName)
{
    std::ifstream ifs(confFileName);
    if (!ifs.is_open())
    {
        LOG_ERR("Unable to open {}", confFileName);
        return false;
    }

    rapidjson::IStreamWrapper isw(ifs);

    rapidjson::Document d;
    d.ParseStream(isw);
    if (d.HasParseError())
    {
        LOG_ERR("Error {} parsing config file '{}' at offset {}", d.GetParseError(), confFileName, d.GetErrorOffset());
        return false;
    }

    m_configName = utl::GetValueOpt<std::string>(d, "configName").value_or(std::string());
    m_fileType = utl::GetValueOpt<tp::FileType>(d, "fileType").value_or(tp::FileType::Text);
    m_regexPattern = utl::GetValueOpt<std::string>(d, "regexPattern").value_or(std::string());

    if (const auto &colsIt = d.FindMember("columns"); colsIt != d.MemberEnd())
    {
        for (const auto &col : colsIt->value.GetArray())
        {
            tp::Column column;
            column.key = utl::GetValueOpt<std::string>(col, "key").value_or(std::string());
            column.name = utl::GetValueOpt<std::string>(col, "name").value_or(std::string());
            column.type = utl::GetValueOpt<tp::ColumnType>(col, "type").value_or(tp::ColumnType::Str);
            column.format = utl::GetValueOpt<std::string>(col, "format").value_or(std::string());
            column.width = utl::GetValueOpt<tp::SInt>(col, "width").value_or(-1L);
            column.pos = utl::GetValueOpt<tp::SInt>(col, "pos").value_or(-1L);
            m_columns.emplace_back(std::move(column));
        }
    }

    if (const auto &highsIt = d.FindMember("highlighters"); highsIt != d.MemberEnd())
    {
        for (const auto &h : highsIt->value.GetArray())
        {
            tp::HighlighterParam hParam;
            hParam.searchParam.column = utl::GetValueOpt<tp::UInt>(h, "column");
            hParam.searchParam.type = utl::GetValueOpt<tp::SearchType>(h, "type").value_or(tp::SearchType::SubString);
            hParam.searchParam.flags = utl::GetValueOpt<tp::SearchFlags>(h, "options").value_or(tp::SearchFlags());
            hParam.searchParam.pattern = utl::GetValueOpt<std::string>(h, "pattern").value_or(std::string());
            hParam.bgColor = utl::GetValueOpt<std::string>(h, "backColor").value_or("White").c_str();
            hParam.textColor = utl::GetValueOpt<std::string>(h, "textColor").value_or("Black").c_str();
            m_highlighterParams.emplace_back(std::move(hParam));
        }
    }

    m_confFileName = confFileName;

    return true;
}

void Conf::saveConfAs(const std::string &confFileName)
{
    const auto &jDoc = toJson();
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    jDoc.Accept(writer);

    std::ofstream ofs(confFileName);
    if (!ofs.is_open())
    {
        LOG_ERR("Unable to open {}", confFileName);
        return;
    }

    ofs << buffer.GetString();
    ofs.flush();
    ofs.close();
    m_confFileName = confFileName;
}

void Conf::saveConf()
{
    if (m_confFileName.empty())
    {
        LOG_ERR("No config file defined");
        return;
    }
    saveConfAs(m_confFileName);
}

void Conf::fromJson(const rapidjson::Document &doc)
{
}

rapidjson::Document Conf::toJson() const
{
    rapidjson::Document jDoc(rapidjson::kObjectType);
    auto &alloc = jDoc.GetAllocator();

    jDoc.AddMember("configName", m_configName, alloc);
    jDoc.AddMember("fileType", tp::toStr(m_fileType), alloc);
    jDoc.AddMember("regexPattern", m_regexPattern, alloc);

    rapidjson::Value jCols(rapidjson::kArrayType);
    for (const auto &column : m_columns)
    {
        rapidjson::Value jCol(rapidjson::kObjectType);
        jCol.AddMember("pos", column.pos, alloc);
        jCol.AddMember("key", column.key, alloc);
        jCol.AddMember("name", column.name, alloc);
        jCol.AddMember("type", tp::toStr(column.type), alloc);
        jCol.AddMember("format", column.format, alloc);
        jCol.AddMember("width", column.width, alloc);
        jCols.GetArray().PushBack(jCol, alloc);
    }
    jDoc.AddMember("columns", jCols, alloc);

    rapidjson::Value jHighlighters(rapidjson::kArrayType);
    for (const auto &hParam : m_highlighterParams)
    {
        rapidjson::Value jHigh(rapidjson::kObjectType);
        jHigh.AddMember("textColor", utl::toStr(hParam.textColor), alloc);
        jHigh.AddMember("backColor", utl::toStr(hParam.bgColor), alloc);
        jHigh.AddMember("type", tp::toStr(hParam.searchParam.type), alloc);
        jHigh.AddMember("options", tp::toStr(hParam.searchParam.flags), alloc);
        jHigh.AddMember("pattern", hParam.searchParam.pattern, alloc);
        if (hParam.searchParam.column.has_value())
            jHigh.AddMember("column", hParam.searchParam.column.value().idx, alloc);
        jHighlighters.GetArray().PushBack(jHigh, alloc);
    }
    jDoc.AddMember("highlighters", jHighlighters, alloc);

    return jDoc;
}