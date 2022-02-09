// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#pragma once

#include "GuiTypes.h"
#include <QFont>
#include <QIcon>

class Style
{
public:
    static void initStyle();
    static QStringList availableStyles();
    static void loadStyleSheet(const QString &fileName);
    static void loadStyle(const QString &styleName);
    static const QFont &getFont() { return inst().m_font; }
    static QIcon getIcon(const QString &iconName) { return QIcon(inst().m_imgDir.absoluteFilePath(iconName)); };
    static const SectionColor &getTextAreaColor() { return inst().m_textAreaColor; }
    static const SectionColor &getSelectedColor() { return inst().m_selectedColor; }
    static const SectionColor &getSelectedTextMarkColor() { return inst().m_selectedTextMarkColor; }
    static const SectionColor &getHeaderColor() { return inst().m_headerColor; }
    static const SectionColor &getBookmarkColor() { return inst().m_bookmarkColor; }
    static const SectionColor &getScrollBarColor() { return inst().m_scrollBarColor; }
    static tp::SInt getTextPadding() { return inst().m_textPadding; }
    static tp::SInt getColumnMargin() { return inst().m_columnMargin; }
    static tp::SInt getScrollBarThickness() { return inst().m_scrollBarThickness; }

    static tp::SInt getTextHeight(bool addPadding);
    static tp::SInt getCharWidth();
    static double getCharWidthF();
    static tp::SInt getTextWidth(const QString &text);
    static double getTextWidthF(const QString &text);
    static QString getElidedText(const QString &text, tp::SInt width, Qt::TextElideMode elideMode, int flags = 0);

private:
    Style() = default;
    static Style &inst();
    void loadStyleConf(const rapidjson::Value &jsonObj);

    QFont m_font;
    QMap<QString, QString> m_availableStyles;
    SectionColor m_textAreaColor;
    SectionColor m_selectedColor;
    SectionColor m_selectedTextMarkColor;
    SectionColor m_headerColor;
    SectionColor m_bookmarkColor;
    SectionColor m_scrollBarColor;
    QString m_style;
    QString m_baseStyle;
    QString m_styleSheet;
    QDir m_imgDir;
    QPalette m_palette;
    tp::SInt m_textPadding = -1;
    tp::SInt m_columnMargin = -1;
    tp::SInt m_scrollBarThickness = -1;
};