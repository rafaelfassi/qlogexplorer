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

    void fromJson(const rapidjson::Document &jDoc);
    rapidjson::Document toJson() const;

    // Call this function before some action that may change the index of the columns
    void fillFkColumnKeys();
    // Call this function after some action that may have changed the index of the columns
    void remapFkColumnFromKeys();

    const std::string &getFileName() const { return m_fileName; }
    void setFileName(const std::string &fileName) { m_fileName = fileName; }
    tp::FileType getFileType() const { return m_fileType; }
    const std::string &getConfigName() const { return m_configName; }
    const std::string &getConfFileName() const { return m_confFileName; }
    const std::string &getRegexPattern() const { return m_regexPattern; }
    void setRegexPattern(const std::string &pattern) { m_regexPattern = pattern; }
    tp::SInt getNoMatchColumn() const { return m_noMatchColumn; }
    void setNoMatchColumn(tp::SInt columnIdx) { m_noMatchColumn = columnIdx; }
    bool exists() const { return !m_confFileName.empty(); }
    void setConfigName(const std::string &configName) { m_configName = configName; }
    std::string getTemplateNameOrType() const;

    tp::Columns &getColumns() { return m_columns; }
    void clearColumns() { m_columns.clear(); }
    void addColumn(tp::Column &&column) { m_columns.emplace_back(column); }
    bool hasDefinedColumns() const { return (!m_columns.empty() && !m_columns.front().key.empty()); }
    bool hasDefinedColumn(tp::SInt columnIdx) const;

    tp::HighlighterParams &getHighlighterParams() { return m_highlighterParams; }
    void clearHighlighterParams() { m_highlighterParams.clear(); }
    void addHighlighterParam(tp::HighlighterParam &&hlt) { m_highlighterParams.emplace_back(hlt); }
    bool hasHighlighterParams() const { return !m_highlighterParams.empty(); }
    bool hasHighlighterParam(tp::SInt hltIdx) const;

    tp::FilterParams &getFilterParams() { return m_filterParams; }
    void clearFilterParams() { m_filterParams.clear(); }
    void addFilterParam(tp::FilterParam &&flt) { m_filterParams.emplace_back(flt); }
    bool hasFilterParams() const { return !m_filterParams.empty(); }
    bool hasFilterParam(tp::SInt fltIdx) const;

    bool isNull() const { return (m_fileType == tp::FileType::None); }
    bool isEqual(const FileConf::Ptr &other) const;
    bool isSameType(const FileConf::Ptr &other) const;
    bool isSameFileAndType(const FileConf::Ptr &other) const;
    bool isDirt(const FileConf::Ptr &other) const;
    void copyFrom(const FileConf::Ptr &other);
    void copyTypeFrom(const FileConf::Ptr &other);
    void copyFileAndTypeFrom(const FileConf::Ptr &other);

    friend bool hasEqualConf(const FileConf &lhs, const FileConf &rhs);
    friend bool operator==(const FileConf &lhs, const FileConf &rhs);

private:
    tp::FileType m_fileType;
    std::string m_configName;
    std::string m_confFileName;
    std::string m_fileName;
    std::string m_regexPattern;
    tp::Columns m_columns;
    tp::HighlighterParams m_highlighterParams;
    tp::FilterParams m_filterParams;
    tp::SInt m_noMatchColumn = 0;
};

inline bool hasEqualConf(const FileConf &lhs, const FileConf &rhs)
{
    return (lhs.m_fileType == rhs.m_fileType) && (lhs.m_configName == rhs.m_configName) &&
           (lhs.m_confFileName == rhs.m_confFileName) && (lhs.m_regexPattern == rhs.m_regexPattern) &&
           (lhs.m_columns == rhs.m_columns) && (lhs.m_highlighterParams == rhs.m_highlighterParams) &&
           (lhs.m_filterParams == rhs.m_filterParams) && (lhs.m_noMatchColumn == rhs.m_noMatchColumn);
}

inline bool operator==(const FileConf &lhs, const FileConf &rhs)
{
    return (lhs.m_fileName == rhs.m_fileName) && hasEqualConf(lhs, rhs);
}
