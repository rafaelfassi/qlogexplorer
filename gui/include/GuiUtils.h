// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "GuiTypes.h"

namespace gutl
{

inline std::string toStr(const QColor &color)
{
    return utl::toStr(color.name());
}

inline QColor toQColor(std::string_view colorName)
{
    return utl::toQStr(colorName);
}

} // namespace gutl
