// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

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

void Style::loadStyleSheetFile(const QString &fileName)
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

void Style::loadStyleFromJson(const rapidjson::Value &jsonObj)
{
    if (jsonObj.MemberCount() == 0)
    {
        return;
    }

    const std::vector<std::pair<std::string, QPalette::ColorGroup>> groups =
        {{"", QPalette::All}, {"Act", QPalette::Active}, {"Ina", QPalette::Inactive}, {"Dsb", QPalette::Disabled}};

    const auto loadColorRole = [&jsonObj, &groups, this](const std::string &key, QPalette::ColorRole role)
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

    const auto loadColorSection = [&jsonObj](const std::string &key, tp::SectionColor &colorSec)
    {
        const auto fg = utl::GetValueOpt<std::string>(jsonObj, fmt::format("{}Fg", key));
        if (fg.has_value())
        {
            colorSec.fg = fg.value().c_str();
        }

        const auto bg = utl::GetValueOpt<std::string>(jsonObj, fmt::format("{}Bg", key));
        if (bg.has_value())
        {
            colorSec.bg = bg.value().c_str();
        }
    };

    const auto loadInt = [&jsonObj](const std::string &key, tp::SInt &intVal)
    {
        const auto val = utl::GetValueOpt<tp::SInt>(jsonObj, key);
        if (val.has_value())
        {
            intVal = val.value();
        }
    };

    const auto qtStyle = utl::GetValueOpt<std::string>(jsonObj, "qtStyle");
    if (qtStyle.has_value())
    {
        m_qtStyle = qtStyle.value().c_str();
    }

    const auto qtStyleSheet = utl::GetValueOpt<std::string>(jsonObj, "qtStyleSheet");
    if (qtStyleSheet.has_value())
    {
        m_qtStyleSheet = qtStyleSheet.value().c_str();
    }

    const auto imgDirStr = utl::GetValueOpt<std::string>(jsonObj, "imgDir");
    if (imgDirStr.has_value())
    {
        QDir imgDir(imgDirStr.value().c_str());
        if (imgDir.exists())
        {
            m_imgDir = imgDir;
        }
        else
        {
            LOG_ERR("Image dir '{}' does not exist", imgDirStr.value());
        }
    }

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

    // Load application specific colors
    loadColorSection("textAreaColor", m_textAreaColor);
    loadColorSection("selectedColor", m_selectedColor);
    loadColorSection("headerColor", m_headerColor);
    loadColorSection("scrollBarColor", m_scrollBarColor);
    loadColorSection("selectedTextMarkColor", m_selectedTextMarkColor);
    loadColorSection("bookmarkColor", m_bookmarkColor);

    // Load integers
    loadInt("textPadding", m_textPadding);
    loadInt("columnMargin", m_columnMargin);
    loadInt("scrollBarThickness", m_scrollBarThickness);
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

    std::string osKey;

#ifdef Q_OS_WIN
    osKey = "osWin";
#elif defined(Q_OS_MAC)
    osKey = "osMac";
#elif defined(Q_OS_LINUX)
    osKey = "osLinux";
#endif

    style.m_palette = qApp->palette();

    style.loadStyleFromJson(d);
    if (!osKey.empty() && d.HasMember(osKey))
    {
        style.loadStyleFromJson(d[osKey]);
    }

    // If nothing was defined by the style file, load colors from palette roles.
    const auto loadColorSectionFromRoles =
        [&style](tp::SectionColor &colorSec, QPalette::ColorRole fg, QPalette::ColorRole bg)
    {
        if (!colorSec.fg.isValid() || !colorSec.bg.isValid())
        {
            colorSec = tp::SectionColor(style.m_palette.color(fg), style.m_palette.color(bg));
        }
    };
    loadColorSectionFromRoles(style.m_textAreaColor, QPalette::Text, QPalette::Base);
    loadColorSectionFromRoles(style.m_selectedColor, QPalette::HighlightedText, QPalette::Highlight);
    loadColorSectionFromRoles(style.m_headerColor, QPalette::WindowText, QPalette::Window);
    loadColorSectionFromRoles(style.m_scrollBarColor, QPalette::Mid, QPalette::Midlight);

    // If nothing was defined by the style file or palette roles, load default colors.
    const auto loadDefaultColorSection = [](tp::SectionColor &colorSec, const QString &defFg, const QString &defBg)
    {
        if (!colorSec.fg.isValid() || !colorSec.bg.isValid())
        {
            colorSec.fg = defFg;
            colorSec.bg = defBg;
        }
    };
    loadDefaultColorSection(style.m_textAreaColor, "#000000", "#FFFFFF");
    loadDefaultColorSection(style.m_selectedColor, "#FFFFFF", "#346792");
    loadDefaultColorSection(style.m_headerColor, "#FFFFFF", "#808080");
    loadDefaultColorSection(style.m_scrollBarColor, "#808080", "#C0C0C0");
    loadDefaultColorSection(style.m_selectedTextMarkColor, "#E0E1E3", "#008000");
    loadDefaultColorSection(style.m_bookmarkColor, "#E0E1E3", "#9400d3");

    // If nothing was defined by the style file, load default integer values.
    const auto loadDefaultInt = [](tp::SInt &intVal, const tp::SInt def)
    {
        if (intVal == -1)
        {
            intVal = def;
        }
    };
    loadDefaultInt(style.m_textPadding, 5);
    loadDefaultInt(style.m_columnMargin, 10);
    loadDefaultInt(style.m_scrollBarThickness, 25);

    style.m_style = styleName;
    const auto qtStyle = style.m_qtStyle.isEmpty() ? style.m_style : style.m_qtStyle;
    if (QStyleFactory::keys().contains(qtStyle))
    {
        qApp->setStyle(QStyleFactory::create(qtStyle));
    }

    if (!style.m_qtStyleSheet.isEmpty())
    {
        loadStyleSheetFile(style.m_qtStyleSheet);
    }

    qApp->setPalette(style.m_palette);
    qApp->setFont(getFont());
}

QIcon Style::makeIcon(const QColor &color, int w, int h)
{
    QPixmap pixmap(w, h);
    pixmap.fill(color);
    return QIcon(pixmap);
}

const QFont &Style::getFont()
{
    return Settings::getFont();
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
    QFontMetricsF fm(getFont());
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