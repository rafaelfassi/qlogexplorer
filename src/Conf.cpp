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

    if (const auto &it = d.FindMember("configName"); it != d.MemberEnd())
    {
        m_configName = it->value.GetString();
    }

    if (const auto &it = d.FindMember("fileType"); it != d.MemberEnd())
    {
        m_fileType = tp::fromStr<tp::FileType>(it->value.GetString());
    }

    if (const auto &it = d.FindMember("regexPattern"); it != d.MemberEnd())
    {
        m_regexPattern = it->value.GetString();
    }

    if (const auto &colsIt = d.FindMember("columns"); colsIt != d.MemberEnd())
    {
        for (const auto &col : colsIt->value.GetArray())
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
            if (const auto &cIt = col.FindMember("width"); cIt != col.MemberEnd())
            {
                column.width = cIt->value.GetInt64();
            }
            if (const auto &cIt = col.FindMember("pos"); cIt != col.MemberEnd())
            {
                column.pos = cIt->value.GetInt64();
            }
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
        jCol.AddMember("width", column.width, alloc);
        jCols.GetArray().PushBack(jCol, alloc);
    }
    jDoc.AddMember("columns", jCols, alloc);

    return jDoc;
}