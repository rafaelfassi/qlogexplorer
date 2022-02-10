// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

// STL
#include <cstdint>
#include <queue>
#include <string>
#include <vector>
#include <optional>
#include <bitset>
#include <set>
#include <map>
#include <memory>
#include <fstream>
#include <limits>
#include <cmath>

// Qt
#include <QString>
#include <QPalette>
#include <QColor>
#include <QRect>
#include <QDir>
#include <QFile>
#include <QVariant>
#include <QDateTime>
#include <QElapsedTimer>
#include <QRegularExpression>

// rapidjson
#define RAPIDJSON_HAS_STDSTRING 1
#include <3rdparty/rapidjson/document.h>
#include <3rdparty/rapidjson/stringbuffer.h>
#include <3rdparty/rapidjson/writer.h>
#include <3rdparty/rapidjson/prettywriter.h>
#include <3rdparty/rapidjson/istreamwrapper.h>

// fmt
#define FMT_HEADER_ONLY 1
#include <3rdparty/fmt/core.h>

// global includes
#include "Types.h"
#include "Utils.h"
#include "Conf.h"
