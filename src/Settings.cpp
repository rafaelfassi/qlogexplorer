// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "Settings.h"
#include <QSettings>
#include <QTranslator>
#include <QApplication>
#include <QStandardPaths>
#include <QFontDatabase>

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
    s.m_translator = new QTranslator();

    const auto &settingsFile = s.m_settingsDir.absoluteFilePath("settings.ini");
    s.m_settings = new QSettings(settingsFile, QSettings::IniFormat);

    QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
    s.loadLanguage();
    s.loadFont();
    s.loadSingleInstance();
    s.loadHideUniqueTab();
    s.loadDefaultSearchType();

    loadTemplates();
}

void Settings::loadLanguage()
{
    m_language = m_settings->value("language", QString()).toString();
    if (m_language.isEmpty())
    {
        const auto sysLang = QLocale::system().language();
        m_language = QLocale::languageToString(sysLang);
    }

    if (m_translator->load(m_language, ":/languages"))
    {
        qApp->installTranslator(m_translator);
    }
    else
    {
        m_language = "en";
    }
}

QStringList Settings::availableLangs()
{
    QStringList langs;
    langs << "en";
    QDir langDir(":/languages");
    for (const auto &langFile : langDir.entryList())
    {
        QFileInfo info(langFile);
        langs << info.baseName();
    }
    return langs;
}

QString Settings::getLanguage()
{
    return inst().m_language;
}

void Settings::setLanguage(const QString &lang)
{
    inst().m_settings->setValue("language", lang);
    inst().loadLanguage();
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

QDir Settings::gettemplatesDir()
{
    return inst().m_templatesDir;
}

tp::SInt Settings::getMaxRecentFiles()
{
    return inst().m_settings->value("maxRecentFiles", 10).toInt();
}

std::vector<FileConf::Ptr> Settings::getRecentFiles()
{
    std::vector<FileConf::Ptr> recentFiles;
    const auto files = inst().m_settings->value("recentFileList").toStringList();
    for (const auto &file : files)
    {
        const auto &recentFileParts = utl::split(utl::toStr(file), "|");
        const auto &fileType = recentFileParts[0];
        const auto &fileName = recentFileParts[1];
        const auto &templFileName = recentFileParts[2];

        if (!QFile::exists(fileName.c_str()))
        {
            // File was deleted.
            LOG_WAR("File '{}' not found", fileName);
            continue;
        }

        auto conf = FileConf::make(tp::fromStr<tp::FileType>(fileType));

        if (!templFileName.empty())
        {
            const auto templCong = Settings::findConfByTemplateFileName(templFileName);
            if (!templCong)
            {
                // Template was deleted.
                LOG_WAR("Template file '{}' not found", templFileName);
                continue;
            }
            conf->copyTypeFrom(templCong);
        }

        conf->setFileName(fileName);
        recentFiles.emplace_back(conf);
    }
    return recentFiles;
}

void Settings::setRecentFile(const FileConf::Ptr &conf)
{
    const auto makeRecentFileStr = [](const FileConf::Ptr &c) -> QString
    {
        const auto &fileType = tp::toStr<tp::FileType>(c->getFileType());
        const auto &fileName = c->getFileName();
        const auto &templateFileName = c->getConfFileName();
        return utl::join({fileType, fileName, templateFileName}, "|").c_str();
    };

    QStringList files;
    const auto &newRecentFileStr = makeRecentFileStr(conf);
    files.append(newRecentFileStr);

    const auto maxReccentFiles = getMaxRecentFiles();
    const auto &recentFiles = getRecentFiles();
    for (const auto &recentFileConf : recentFiles)
    {
        const auto &recentFileStr = makeRecentFileStr(recentFileConf);
        if (recentFileStr != newRecentFileStr)
        {
            files.append(recentFileStr);
            if (files.size() >= maxReccentFiles)
                break;
        }
    }

    inst().m_settings->setValue("recentFileList", files);
}

FileConf::Ptr Settings::findConfByTemplateFileName(const std::string &templateFileName)
{
    const auto &templates(inst().m_templates);

    const auto it = std::find_if(
        templates.begin(),
        templates.end(),
        [&templateFileName](const FileConf::Ptr &conf) { return (conf->getConfFileName() == templateFileName); });

    if (it != templates.end())
    {
        return *it;
    }

    return nullptr;
}

FileConf::Ptr Settings::findConfByTemplateName(const std::string &name)
{
    const auto &templates(inst().m_templates);

    const auto it = std::find_if(
        templates.begin(),
        templates.end(),
        [&name](const FileConf::Ptr &conf) { return (conf->getConfigName() == name); });

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

    confTemplates.clear();

    const auto &templateFiles = templDir.entryList(QDir::Files | QDir::Readable);
    for (const auto &templ : templateFiles)
    {
        const auto &templFileName = templDir.absoluteFilePath(templ).toStdString();
        // Ensures no duplicated template
        if (!findConfByTemplateFileName(templFileName))
        {
            confTemplates.push_back(FileConf::make(templFileName));
        }
    }
}

void Settings::saveTemplate(FileConf::Ptr conf)
{
    conf->saveConf();
    auto templConf = findConfByTemplateFileName(conf->getConfFileName());
    if (!templConf)
    {
        LOG_ERR("Template {} not found", conf->getFileName());
        return;
    }
    templConf->copyFrom(conf);
}

void Settings::saveTemplateAs(FileConf::Ptr conf, const QString &name)
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
    fileName.append(idx);
    if (name.isEmpty())
        conf->setConfigName(utl::toStr(fileName));
    else
        conf->setConfigName(utl::toStr(name));
    fileName.append(ext);

    fileName = templDir.absoluteFilePath(fileName);
    conf->saveConfAs(fileName.toStdString());
    confTemplates.push_back(FileConf::clone(conf));
}

void Settings::deleteTemplate(FileConf::Ptr conf)
{
    if (!conf->exists())
        return;

    if (QFile::remove(conf->getConfFileName().c_str()))
    {
        auto &confTemplates(inst().m_templates);
        confTemplates.erase(
            std::remove_if(
                confTemplates.begin(),
                confTemplates.end(),
                [&conf](const FileConf::Ptr &c) { return c->isSameType(conf); }),
            confTemplates.end());
    }
}

std::vector<FileConf::Ptr> Settings::getTemplates()
{
    return inst().m_templates;
}

void Settings::setStyle(const QString styleName)
{
    inst().m_settings->setValue("styleName", styleName);
}

QString Settings::getStyle()
{
    return inst().m_settings->value("styleName", "Dark").toString();
}

void Settings::loadFont()
{
    const auto fontFamily = m_settings->value("fontFamily", "DejaVu Sans Mono").toString();
    const auto fontSize = m_settings->value("fontSize", 12).toInt();
    m_font = QFont(fontFamily, fontSize);
}

void Settings::setFont(const QString &family, int size)
{
    Settings &s = inst();
    s.m_settings->setValue("fontFamily", family);
    s.m_settings->setValue("fontSize", size);
    s.loadFont();
}

void Settings::loadSingleInstance()
{
    m_singleInstance = m_settings->value("singleInstance", true).toBool();
}

void Settings::setSingleInstance(bool singleInstance)
{
    inst().m_settings->setValue("singleInstance", singleInstance);
    inst().loadSingleInstance();
}

bool Settings::getSingleInstance()
{
    return inst().m_singleInstance;
}

void Settings::loadHideUniqueTab()
{
    m_hideUniqueTab = inst().m_settings->value("hideUniqueTab", false).toBool();
}

void Settings::setHideUniqueTab(bool hideTab)
{
    inst().m_settings->setValue("hideUniqueTab", hideTab);
    inst().loadHideUniqueTab();
}

bool Settings::getHideUniqueTab()
{
    return inst().m_hideUniqueTab;
}

void Settings::loadDefaultSearchType()
{
    const auto searchTypeStr =
        inst().m_settings->value("defaultSearchType", tp::toStr(tp::SearchType::Regex).c_str()).toString();
    m_searchType = tp::fromStr<tp::SearchType>(utl::toStr(searchTypeStr));
}

void Settings::setDefaultSearchType(tp::SearchType searchType)
{
    const QString searchTypeStr(tp::toStr<tp::SearchType>(searchType).c_str());
    inst().m_settings->setValue("defaultSearchType", searchTypeStr);
    inst().loadDefaultSearchType();
}

tp::SearchType Settings::getDefaultSearchType()
{
    return inst().m_searchType;
}
