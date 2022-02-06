// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#include "pch.h"
#include "Style.h"
#include <QFontMetrics>

Style &Style::inst()
{
    static Style style;
    return style;
}

QStringList Style::availableStyles()
{
    QDir stylesDir(":/styles");
    QStringList filters;
    filters << "*.json";
    stylesDir.setNameFilters(filters);

    QStringList styles;
    for (const auto &stInfo : stylesDir.entryInfoList())
    {
        styles.append(stInfo.baseName());
    }
    return styles;
}

QString Style::loadStyle(const QString &styleName)
{
    Style &style = inst();

    QFile qssFile(QString(":/styles/%1.qss").arg(styleName));
    if (!qssFile.exists())
    {
        LOG_ERR("Template '{}' not found", qssFile.fileName().toStdString());
        return {};
    }
    if (!qssFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        LOG_ERR("Cannot open template '{}'", qssFile.fileName().toStdString());
        return {};
    }
    QString qss = qssFile.readAll();

    QFile stFile(QString(":/styles/%1.json").arg(styleName));
    if (!stFile.exists())
    {
        LOG_ERR("Style '{}' not found", stFile.fileName().toStdString());
        return {};
    }
    if (!stFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        LOG_ERR("Cannot open style '{}'", stFile.fileName().toStdString());
        return {};
    }
    QString st = stFile.readAll();

    rapidjson::Document d;
    d.Parse(st.toStdString());
    if (d.HasParseError())
    {
        LOG_ERR(
            "Error {} parsing style file '{}' at offset {}",
            d.GetParseError(),
            stFile.fileName().toStdString(),
            d.GetErrorOffset());
        return {};
    }

    // Replace Qt StyleSheet template with the style values
    for (auto i = d.MemberBegin(); i != d.MemberEnd(); ++i)
    {
        QString key = QString("<%1>").arg(i->name.GetString());
        QString val;

        if (i->value.IsString())
            val = i->value.GetString();
        else if (i->value.IsNumber())
            val = QString::number(i->value.GetInt64());

        if (!val.isEmpty())
            qss.replace(key, val);
    }

    const auto loadColorSection = [&d](const std::string &key, SectionColor &colorSec)
    {
        const auto fg = utl::GetValueOpt<std::string>(d, fmt::format("{}Fg", key));
        const auto bg = utl::GetValueOpt<std::string>(d, fmt::format("{}Bg", key));

        if (fg.has_value())
        {
            colorSec.fg = fg.value().c_str();
        }
        else
        {
            LOG_ERR("Foreground color not found for {}", key);
        }

        if (bg.has_value())
        {
            colorSec.bg = bg.value().c_str();
        }
        else
        {
            LOG_ERR("Background color not found for {}", key);
        }
    };

    const auto loadInt = [&d](const std::string &key, tp::SInt &intVal)
    {
        const auto val = utl::GetValueOpt<tp::SInt>(d, key);
        if (val.has_value())
        {
            intVal = val.value();
        }
        else
        {
            LOG_ERR("Int val not found for {}", key);
        }
    };

    // Load font
    const auto fontName = utl::GetValueOpt<std::string>(d, "fontName");
    const auto fontSize = utl::GetValueOpt<tp::SInt>(d, "fontSize");
    style.m_font = QFont(fontName.value_or("DejaVu Sans Mono").c_str(), fontSize.value_or(12));

    // Load colors
    loadColorSection("textAreaColor", style.m_textAreaColor);
    loadColorSection("selectedColor", style.m_selectedColor);
    loadColorSection("selectedTextMarkColor", style.m_selectedTextMarkColor);
    loadColorSection("headerColor", style.m_headerColor);
    loadColorSection("bookmarkColor", style.m_bookmarkColor);
    loadColorSection("scrollBarColor", style.m_scrollBarColor);

    // Load integers
    loadInt("textPadding", style.m_textPadding);
    loadInt("columnMargin", style.m_columnMargin);
    loadInt("scrollBarThickness", style.m_scrollBarThickness);

    return qss;
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