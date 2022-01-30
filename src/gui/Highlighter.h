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
