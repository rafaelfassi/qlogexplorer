#pragma once

class Conf
{
public:
    Conf(tp::FileType fileType = tp::FileType::None);
    Conf(const std::string& confFileName);
    bool loadFConf(const std::string& confFileName);
    void saveConfAs(const std::string& confFileName);
    void saveConf();

    void fromJson(const rapidjson::Document& doc);
    rapidjson::Document toJson() const;

    const std::string& getFileName() const { return m_fileName; }
    void setFileName(const std::string& fileName) { m_fileName = fileName; }
    tp::FileType getFileType() const { return m_fileType; }
    const std::string& getConfigName() const { return m_configName; }
    const std::string& getConfFileName() const { return m_confFileName; }
    bool exists() const { return !m_confFileName.empty(); }
    void setConfigName(const std::string& configName) { m_configName = configName; }
    tp::Columns& getColumns() { return m_columns; }
    void addColumn(tp::Column &&column) { m_columns.emplace_back(column); }
    

private:
    tp::FileType m_fileType;
    std::string m_configName;
    std::string m_confFileName;
    std::string m_fileName;
    std::string regexPattern;
    tp::Columns m_columns;
    tp::HighlighterParams highlighterParams;
    
};

Q_DECLARE_METATYPE(Conf*)