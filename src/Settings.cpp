// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#include "pch.h"
#include "Settings.h"
#include <QSettings>
#include <QStandardPaths>

Settings &Settings::inst()
{
    static Settings settings;
    return settings;
}

void Settings::initSettings()
{
    Settings &s = inst();

    const auto &confPaths = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    if (confPaths.isEmpty())
    {
        LOG_ERR("Cannot locate the default configuration directory");
        return;
    }

    s.m_settingsDir.setPath(confPaths.first());
    if (!s.m_settingsDir.exists())
    {
        if (!s.m_settingsDir.mkpath(s.m_settingsDir.absolutePath()))
        {
            LOG_ERR("Cannot create config directory {}", utl::toStr(s.m_settingsDir.absolutePath()));
            return;
        }
    }

    s.m_templatesDir.setPath(s.m_settingsDir.absolutePath() + QDir::separator() + "templates");
    if (!s.m_templatesDir.exists())
    {
        if (!s.m_templatesDir.mkpath(s.m_templatesDir.absolutePath()))
        {
            LOG_ERR("Cannot create templates directory {}", utl::toStr(s.m_templatesDir.absolutePath()));
            return;
        }
    }

    s.m_settings = new QSettings();

    const auto &settingsFile = s.m_settingsDir.absoluteFilePath("settings.ini");
    s.m_settings = new QSettings(settingsFile, QSettings::IniFormat);

    loadTemplates();
}

QDir Settings::getSettingsDir(const QString &subDir)
{
    QDir dir = inst().m_settingsDir;
    if (!subDir.isEmpty())
    {
        dir.setPath(dir.absolutePath() + QDir::separator() + subDir);
    }
    return dir;
}

tp::SInt Settings::getMaxRecentFiles()
{
    return inst().m_settings->value("maxRecentFiles", 10).toInt();
}

std::vector<Conf> Settings::getRecentFiles()
{
    std::vector<Conf> recentFiles;
    const auto files = inst().m_settings->value("recentFileList").toStringList();
    for (const auto &file : files)
    {
        const auto &recentFileParts = utl::split(utl::toStr(file), "|");
        const auto &fileType = recentFileParts[0];
        const auto &fileName = recentFileParts[1];
        const auto &templFileName = recentFileParts[2];

        Conf conf(tp::fromStr<tp::FileType>(fileType));

        if (!templFileName.empty())
        {
            const auto templCong = Settings::findConfByTemplateFileName(templFileName);
            if (templCong == nullptr)
            {
                // Template was deleted.
                continue;
            }

            conf = *templCong;
        }

        conf.setFileName(fileName);
        recentFiles.emplace_back(conf);
    }
    return recentFiles;
}

void Settings::setRecentFile(Conf *conf)
{
    QStringList files = inst().m_settings->value("recentFileList").toStringList();
    const auto &fileType = tp::toStr<tp::FileType>(conf->getFileType());
    const auto &fileName = conf->getFileName();
    const auto &templateFileName = conf->getConfFileName();

    const QString recentFile = utl::join({fileType, fileName, templateFileName}, "|").c_str();

    files.removeAll(recentFile);
    files.prepend(recentFile);

    const auto maxReccentFiles = inst().getMaxRecentFiles();
    while (files.size() > maxReccentFiles)
    {
        files.removeLast();
    }

    inst().m_settings->setValue("recentFileList", files);
}

Conf *Settings::findConfByTemplateFileName(const std::string &templateFileName)
{
    const auto &templates(inst().m_templates);

    const auto it = std::find_if(
        templates.begin(),
        templates.end(),
        [&templateFileName](const Conf *conf) { return (conf->getConfFileName() == templateFileName); });

    if (it != templates.end())
    {
        return *it;
    }
    return nullptr;
}

void Settings::loadTemplates()
{
    const auto &templDir(inst().m_templatesDir);
    auto &confTemplates(inst().m_templates);

    for (auto conf : confTemplates)
    {
        delete conf;
    }
    confTemplates.clear();

    const auto &templateFiles = templDir.entryList(QDir::Files | QDir::Readable);
    for (const auto &templ : templateFiles)
    {
        const auto &templFileName = templDir.absoluteFilePath(templ).toStdString();
        // Ensures no duplicated template
        if (findConfByTemplateFileName(templFileName) == nullptr)
        {
            Conf *conf = new Conf(templFileName);
            confTemplates.push_back(conf);
        }
    }
}

void Settings::saveTemplate(Conf *conf)
{
    conf->saveConf();
    auto templConf = findConfByTemplateFileName(conf->getConfFileName());
    if (templConf == nullptr)
    {
        LOG_ERR("Template {} not found", conf->getFileName());
        return;
    }
    *templConf = *conf;
}

void Settings::saveTemplateAs(Conf *conf, const QString &name)
{
    const auto &templDir(inst().m_templatesDir);
    auto &confTemplates(inst().m_templates);

    conf->setConfigName(utl::toStr(name));

    QString fileName = name.simplified().toLower().replace(QRegularExpression("[^a-z\\d-]"), "_");
    if (fileName.isEmpty())
    {
        fileName = "template";
    }

    int cnt(0);
    QString idx;
    QString ext(".json");
    while (templDir.exists(fileName + idx + ext))
    {
        idx = QString("_%1").arg(++cnt);
    }
    fileName.append(idx + ext);

    fileName = templDir.absoluteFilePath(fileName);
    conf->saveConfAs(fileName.toStdString());

    confTemplates.push_back(new Conf(*conf));
}

std::vector<Conf *> Settings::getTemplates()
{
    return inst().m_templates;
}

void Settings::setStyle(const QString styleName)
{
    inst().m_settings->setValue("styleName", styleName);
}

QString Settings::getStyle()
{
    return inst().m_settings->value("styleName", "Default").toString();
}