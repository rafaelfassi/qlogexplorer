// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#pragma once

#include "GuiTypes.h"
#include <QFont>

class Style
{
public:
    static QStringList availableStyles();
    static QString loadStyle(const QString &styleName);
    static const QFont &getFont() { return inst().m_font; }
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

    QFont m_font;
    SectionColor m_textAreaColor;
    SectionColor m_selectedColor;
    SectionColor m_selectedTextMarkColor;
    SectionColor m_headerColor;
    SectionColor m_bookmarkColor;
    SectionColor m_scrollBarColor;
    tp::SInt m_textPadding;
    tp::SInt m_columnMargin;
    tp::SInt m_scrollBarThickness;
};