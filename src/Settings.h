// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QFont>

class QSettings;
class QTranslator;

class Settings
{
public:
    static void initSettings();

    static QStringList availableLangs();
    static QString getLanguage();
    static void setLanguage(const QString &lang);

    static QDir getSettingsDir(const QString &subDir = {});
    static QDir gettemplatesDir();

    static tp::SInt getMaxRecentFiles();
    static std::vector<FileConf::Ptr> getRecentFiles();
    static void setRecentFile(const FileConf::Ptr &conf);

    static FileConf::Ptr findConfByTemplateFileName(const std::string &templateFileName);
    static FileConf::Ptr findConfByTemplateName(const std::string &name);
    static void loadTemplates();
    static void saveTemplate(FileConf::Ptr conf);
    static void saveTemplateAs(FileConf::Ptr conf, const QString &name);
    static void deleteTemplate(FileConf::Ptr conf);
    static std::vector<FileConf::Ptr> getTemplates();

    static void setStyle(const QString styleName);
    static QString getStyle();

    static void setFont(const QString &family, int size);
    static const QFont &getFont() { return inst().m_font; }

    static void setSingleInstance(bool singleInstance);
    static bool getSingleInstance();

    static void setHideUniqueTab(bool hideTab);
    static bool getHideUniqueTab();

    static void setDefaultSearchType(tp::SearchType searchType);
    static tp::SearchType getDefaultSearchType();

private:
    Settings() = default;
    static Settings &inst();
    void loadLanguage();
    void loadFont();
    void loadSingleInstance();
    void loadHideUniqueTab();
    void loadDefaultSearchType();

    QFont m_font;
    QDir m_settingsDir;
    QDir m_templatesDir;
    QSettings *m_settings;
    QTranslator *m_translator;
    QString m_language;
    bool m_singleInstance;
    bool m_hideUniqueTab;
    tp::SearchType m_searchType;
    std::vector<FileConf::Ptr> m_templates;
};
