#include "pch.h"
#include "Conf.h"

Conf::Conf(tp::FileType fileType) : m_fileType(fileType)
{
}

Conf::Conf(const std::string& confFileName)
{
    loadFConf(confFileName);
}

bool Conf::loadFConf(const std::string &confFileName)
{
    std::ifstream ifs(confFileName);
    if (!ifs.is_open())
    {
        qCritical() << "Unable to open" << confFileName.c_str();
        return false;
    }

    rapidjson::IStreamWrapper isw(ifs);

    rapidjson::Document d;
    d.ParseStream(isw);
    if (d.HasParseError())
    {
        qCritical() << "Error" << d.GetParseError() << "parsing config file " << confFileName.c_str() << "at offset"
                    << d.GetErrorOffset();
        return false;
    }

    if (const auto &it = d.FindMember("configName"); it != d.MemberEnd())
    {
        m_configName = it->value.GetString();
    }

    if (const auto &it = d.FindMember("fileType"); it != d.MemberEnd())
    {
        tp::fromStr(it->value.GetString(), m_fileType);
    }

    if (const auto &colsIt = d.FindMember("columns"); colsIt != d.MemberEnd())
    {
        for (const auto& col : colsIt->value.GetArray())
        {
            tp::Column column;
            if (const auto &cIt = col.FindMember("key"); cIt != col.MemberEnd())
            {
                column.key = cIt->value.GetString();
            }
            if (const auto &cIt = col.FindMember("name"); cIt != col.MemberEnd())
            {
                column.name = cIt->value.GetString();
            }
            if (const auto &cIt = col.FindMember("fileType"); cIt != col.MemberEnd())
            {
                column.type = tp::fromStr<tp::ColumnType>(cIt->value.GetString());
            }
            // if (const auto &cIt = col.FindMember("Width"); cIt != col.MemberEnd())
            // {
            //     column.Width = cIt->value.GetInt64();
            // }
            m_columns.emplace_back(std::move(column));
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
        qCritical() << "Unable to open" << confFileName.c_str();
        return;
    }

    ofs << buffer.GetString();
    ofs.flush();
    ofs.close();
    m_confFileName = confFileName;
}

void Conf::saveConf() const
{
    
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

    rapidjson::Value jCols(rapidjson::kArrayType);
    for (const auto &column : m_columns)
    {
        rapidjson::Value jCol(rapidjson::kObjectType);
        jCol.AddMember("key", column.key, alloc);
        jCol.AddMember("name", column.name, alloc);
        jCol.AddMember("type", tp::toStr(column.type), alloc);
        jCol.AddMember("Width", column.Width, alloc);
        jCols.GetArray().PushBack(jCol, alloc);
    }
    jDoc.AddMember("columns", jCols, alloc);

    return jDoc;
}