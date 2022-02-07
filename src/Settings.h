// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#pragma once
class QSettings;

class Settings
{
public:
    static void initSettings();

    static tp::SInt getMaxRecentFiles();
    static std::vector<Conf> getRecentFiles();
    static void setRecentFile(Conf *conf);

    static Conf *findConfByTemplateFileName(const std::string &templateFileName);
    static void loadTemplates();
    static void saveTemplate(Conf *conf);
    static void saveTemplateAs(Conf *conf, const QString &name);
    static std::vector<Conf *> getTemplates();

    static void setStyle(const QString styleName);
    static QString getStyle();

private:
    Settings() = default;
    static Settings &inst();

    QDir m_settingsDir;
    QDir m_templatesDir;
    QSettings *m_settings;
    std::vector<Conf *> m_templates;
};