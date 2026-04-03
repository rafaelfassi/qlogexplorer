// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "match/Matcher.h"
#include "GuiTypes.h"

class Highlighter
{
public:
    Highlighter(const tp::HighlighterParam &param);
    bool matchInRow(const tp::RowData &rowData) const;
    const QColor &getTextColor() const { return m_selColor.fg; }
    const QColor &getBgColor() const { return m_selColor.bg; }

private:
    tp::HighlighterParam m_param;
    gtp::SectionColor m_selColor;
    Matcher m_matcher;
};
