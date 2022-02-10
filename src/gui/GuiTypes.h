// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

struct TextCan
{
    TextCan() = default;
    TextCan(const QString &_text) : text(_text) {}
    TextCan(const QRect &_rect, const QString &_text = {}) : rect(_rect), text(_text) {}
    QRect rect;
    QString text;
};

struct SectionColor
{
    SectionColor() = default;
    SectionColor(const QColor &_fg, const QColor &_bg) : fg(_fg), bg(_bg) {}
    QColor fg;
    QColor bg;
};

struct TextSelection
{
    TextSelection() = default;
    TextSelection(const TextCan &_can, const SectionColor &_color) : can(_can), color(_color) {}
    TextCan can;
    SectionColor color;
}; 
