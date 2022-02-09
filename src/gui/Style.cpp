// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#include "pch.h"
#include "Style.h"
#include <QPalette>
#include <QFontMetrics>
#include <QApplication>
#include <QStyleFactory>
#include <Settings.h>

Style &Style::inst()
{
    static Style style;
    return style;
}

void Style::initStyle()
{
    Style &s = inst();

    QStringList qtStyles = QStyleFactory::keys();
    for (const auto qtStyle : qtStyles)
    {
        s.m_availableStyles[qtStyle] = QString();
    }

    QStringList filters = {"*.json"};

    QDir stylesInResDir(":/styles");
    if (stylesInResDir.exists())
    {
        stylesInResDir.setNameFilters(filters);
        for (const auto &stInfo : stylesInResDir.entryInfoList())
        {
            s.m_availableStyles[stInfo.baseName()] = stInfo.filePath();
        }
    }

    QDir customStylesDir = Settings::getSettingsDir("styles");
    if (customStylesDir.exists())
    {
        customStylesDir.setNameFilters(filters);
        for (const auto &stInfo : customStylesDir.entryInfoList())
        {
            s.m_availableStyles[stInfo.baseName()] = stInfo.filePath();
        }
    }

    s.m_imgDir.setPath(":/images/default");
}

QStringList Style::availableStyles()
{
    return inst().m_availableStyles.keys();
}

void Style::loadStyleSheet(const QString &fileName)
{
    QFile qssFile(fileName);
    if (qssFile.exists() && qssFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        const QString qss = qssFile.readAll();
        qApp->setStyleSheet(qss);
    }
    else
    {
        LOG_ERR("Cannot open StyleSheet '{}'", fileName.toStdString());
    }
}

void Style::loadStyleConf(const rapidjson::Value &jsonObj)
{
    static std::vector<std::pair<std::string, QPalette::ColorGroup>> groups =
        {{"", QPalette::All}, {"Atc", QPalette::Active}, {"Ina", QPalette::Inactive}, {"Dsb", QPalette::Disabled}};

    const auto loadColorRole = [&jsonObj, this](const std::string &key, QPalette::ColorRole role)
    {
        for (const auto &[groupStr, groupEnum] : groups)
        {
            const auto cl = utl::GetValueOpt<std::string>(jsonObj, key + groupStr);
            if (cl.has_value())
            {
                m_palette.setColor(groupEnum, role, cl.value().c_str());
            }
        }
    };

    const auto loadColorSection =
        [&jsonObj](const std::string &key, SectionColor &colorSec, const QString &defFg = {}, const QString &defBg = {})
    {
        const auto fg = utl::GetValueOpt<std::string>(jsonObj, fmt::format("{}Fg", key));
        if (fg.has_value())
        {
            colorSec.fg = fg.value().c_str();
        }
        else if (!colorSec.fg.isValid() && !defFg.isEmpty())
        {
            colorSec.fg = defFg;
        }

        const auto bg = utl::GetValueOpt<std::string>(jsonObj, fmt::format("{}Bg", key));
        if (bg.has_value())
        {
            colorSec.bg = bg.value().c_str();
        }
        else if (!colorSec.bg.isValid() && !defBg.isEmpty())
        {
            colorSec.bg = defBg;
        }
    };

    const auto loadColorSectionFromRoles =
        [this](SectionColor &colorSec, QPalette::ColorRole fg, QPalette::ColorRole bg)
    {
        if (!colorSec.fg.isValid() || !colorSec.bg.isValid())
        {
            colorSec = SectionColor(m_palette.color(fg), m_palette.color(bg));
        }
    };

    const auto loadInt = [&jsonObj](const std::string &key, tp::SInt &intVal, const tp::SInt def)
    {
        const auto val = utl::GetValueOpt<tp::SInt>(jsonObj, key);
        if (val.has_value())
        {
            intVal = val.value();
        }
        else if (intVal == -1)
        {
            intVal = def;
        }
    };

    // Load font
    const auto fontName = utl::GetValueOpt<std::string>(jsonObj, "fontName");
    const auto fontSize = utl::GetValueOpt<tp::SInt>(jsonObj, "fontSize");
    m_font = QFont(fontName.value_or("DejaVu Sans Mono").c_str(), fontSize.value_or(12));

    const auto baseStyle = utl::GetValueOpt<std::string>(jsonObj, "style");
    if (baseStyle.has_value())
    {
        m_baseStyle = baseStyle.value().c_str();
    }

    const auto qtStyleSheet = utl::GetValueOpt<std::string>(jsonObj, "qtStyleSheet");
    if (qtStyleSheet.has_value())
    {
        m_styleSheet = qtStyleSheet.value().c_str();
    }

    const auto imgDirStr = utl::GetValueOpt<std::string>(jsonObj, "imgDir");
    if (imgDirStr.has_value())
    {
        QDir imgDir(imgDirStr.value().c_str());
        if (imgDir.exists())
            m_imgDir = imgDir;
        else
            LOG_ERR("Image dir '{}' does not exist", imgDirStr.value());
    }

    if (jsonObj.MemberCount() > 0)
    {
        // Load palette colors
        loadColorRole("qtWindow", QPalette::Window);
        loadColorRole("qtWindowText", QPalette::WindowText);
        loadColorRole("qtBase", QPalette::Base);
        loadColorRole("qtAlternateBase", QPalette::AlternateBase);
        loadColorRole("qtToolTipBase", QPalette::ToolTipBase);
        loadColorRole("qtToolTipText", QPalette::ToolTipText);
        loadColorRole("qtPlaceholderText", QPalette::PlaceholderText);
        loadColorRole("qtText", QPalette::Text);
        loadColorRole("qtButton", QPalette::Button);
        loadColorRole("qtButtonText", QPalette::ButtonText);
        loadColorRole("qtBrightText", QPalette::BrightText);
        loadColorRole("qtLight", QPalette::Light);
        loadColorRole("qtMidlight", QPalette::Midlight);
        loadColorRole("qtDark", QPalette::Dark);
        loadColorRole("qtMid", QPalette::Mid);
        loadColorRole("qtShadow", QPalette::Shadow);
        loadColorRole("qtHighlight", QPalette::Highlight);
        loadColorRole("qtHighlightedText", QPalette::HighlightedText);
        loadColorRole("qtLink", QPalette::Link);
        loadColorRole("qtLinkVisited", QPalette::LinkVisited);
    }

    // Load integers
    loadInt("textPadding", m_textPadding, 5);
    loadInt("columnMargin", m_columnMargin, 10);
    loadInt("scrollBarThickness", m_scrollBarThickness, 25);

    // Load colors from palette
    loadColorSectionFromRoles(m_textAreaColor, QPalette::Text, QPalette::Base);
    loadColorSectionFromRoles(m_selectedColor, QPalette::HighlightedText, QPalette::Highlight);
    loadColorSectionFromRoles(m_headerColor, QPalette::WindowText, QPalette::Window);
    loadColorSectionFromRoles(m_scrollBarColor, QPalette::Mid, QPalette::Midlight);

    // Load application specific colors (may overwrite the ones got from the palette)
    loadColorSection("textAreaColor", m_textAreaColor, "#000000", "#FFFFFF");
    loadColorSection("selectedColor", m_selectedColor, "#FFFFFF", "#346792");
    loadColorSection("headerColor", m_headerColor, "#FFFFFF", "#808080");
    loadColorSection("scrollBarColor", m_scrollBarColor, "#808080", "#C0C0C0");
    loadColorSection("selectedTextMarkColor", m_selectedTextMarkColor, "#E0E1E3", "#008000");
    loadColorSection("bookmarkColor", m_bookmarkColor, "#E0E1E3", "#9400d3");
}

void Style::loadStyle(const QString &styleName)
{
    Style &style = inst();

    QString styleFileName;
    if (style.m_availableStyles.contains(styleName))
    {
        styleFileName = style.m_availableStyles[styleName];
    }
    else
    {
        LOG_ERR("Style '{}' is not available", utl::toStr(styleName));
    }

    std::optional<std::string> styleFileContent;
    if (!styleFileName.isEmpty())
    {
        QFile stFile(styleFileName);
        if (stFile.exists() && stFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            styleFileContent = utl::toStr(stFile.readAll());
            stFile.close();
        }
        else
        {
            LOG_ERR("Style '{}' not found", utl::toStr(styleFileName));
        }
    }

    rapidjson::Document d;
    d.Parse(styleFileContent.value_or("{}"));
    if (d.HasParseError())
    {
        LOG_ERR(
            "Error {} parsing style file '{}' at offset {}",
            d.GetParseError(),
            utl::toStr(styleFileName),
            d.GetErrorOffset());
        return;
    }

    style.m_palette = qApp->palette();

    style.loadStyleConf(d);

    std::string osKey;

#ifdef Q_OS_WIN
    osKey = "osWin";
#elif defined(Q_OS_MAC)
    osKey = "osMac";
#elif defined(Q_OS_LINUX)
    osKey = "osLinux";
#endif

    if (!osKey.empty() && d.HasMember(osKey))
    {
        style.loadStyleConf(d[osKey]);
    }

    style.m_style = styleName;
    const auto qtStyle = style.m_baseStyle.isEmpty() ? style.m_style : style.m_baseStyle;
    if (QStyleFactory::keys().contains(qtStyle))
    {
        qApp->setStyle(QStyleFactory::create(qtStyle));
    }

    if (!style.m_styleSheet.isEmpty())
    {
        loadStyleSheet(style.m_styleSheet);
    }

    qApp->setPalette(style.m_palette);
    qApp->setFont(style.m_font);
}

tp::SInt Style::getTextHeight(bool addPadding)
{
    QFontMetrics fm(getFont());
    return fm.height() + (addPadding ? (2 * getTextPadding()) : 0L);
}

tp::SInt Style::getCharWidth()
{
    QFontMetrics fm(getFont());
    return fm.horizontalAdvance('a');
}

double Style::getCharWidthF()
{
    QFontMetricsF fm(Style::getFont());
    return fm.horizontalAdvance('a');
}

tp::SInt Style::getTextWidth(const QString &text)
{
    QFontMetrics fm(getFont());
    return fm.horizontalAdvance(text);
}

double Style::getTextWidthF(const QString &text)
{
    QFontMetricsF fm(getFont());
    return fm.horizontalAdvance(text);
}

QString Style::getElidedText(const QString &text, tp::SInt width, Qt::TextElideMode elideMode, int flags)
{
    QFontMetrics fm(getFont());
    return fm.elidedText(text, elideMode, width, flags);
}