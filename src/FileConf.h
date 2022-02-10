// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

class FileConf
{
public:
    using Ptr = std::shared_ptr<FileConf>;
    static Ptr make(tp::FileType fileType = tp::FileType::None) { return std::make_shared<FileConf>(fileType); }
    static Ptr make(const std::string &confFileName) { return std::make_shared<FileConf>(confFileName); }
    static Ptr clone(const Ptr &conf) { return std::make_shared<FileConf>(*conf.get()); }

    FileConf(tp::FileType fileType = tp::FileType::None);
    FileConf(const std::string &confFileName);
    bool loadFConf(const std::string &confFileName);
    void saveConfAs(const std::string &confFileName);
    void saveConf();

    void fromJson(const rapidjson::Document &doc);
    rapidjson::Document toJson() const;

    const std::string &getFileName() const { return m_fileName; }
    void setFileName(const std::string &fileName) { m_fileName = fileName; }
    tp::FileType getFileType() const { return m_fileType; }
    const std::string &getConfigName() const { return m_configName; }
    const std::string &getConfFileName() const { return m_confFileName; }
    const std::string &getRegexPattern() const { return m_regexPattern; }
    void setRegexPattern(const std::string &pattern) { m_regexPattern = pattern; }
    bool exists() const { return !m_confFileName.empty(); }
    void setConfigName(const std::string &configName) { m_configName = configName; }
    tp::Columns &getColumns() { return m_columns; }
    void clearColumns() { m_columns.clear(); }
    void addColumn(tp::Column &&column) { m_columns.emplace_back(column); }
    tp::HighlighterParams &getHighlighterParams() { return m_highlighterParams; }

    bool isSameType(const FileConf::Ptr &other) const;
    bool isSameFileAndType(const FileConf::Ptr &other) const;
    void copyFrom(const FileConf::Ptr &other);
    void copyTypeFrom(const FileConf::Ptr &other);
    void copyFileAndTypeFrom(const FileConf::Ptr &other);

private:
    tp::FileType m_fileType;
    std::string m_configName;
    std::string m_confFileName;
    std::string m_fileName;
    std::string m_regexPattern;
    tp::Columns m_columns;
    tp::HighlighterParams m_highlighterParams;
};
