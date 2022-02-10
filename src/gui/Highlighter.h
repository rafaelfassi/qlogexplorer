// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "Matcher.h"

class Highlighter
{
public:
    Highlighter(const tp::HighlighterParam& param);
    bool matchInRow(const tp::RowData &rowData) const;
    QColor getTextColor() const { return m_param.textColor; }
    QColor getBgColor() const { return m_param.bgColor; }

private:
    tp::HighlighterParam m_param;
    Matcher m_matcher;
};
