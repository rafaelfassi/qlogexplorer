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

// Qt Core
#include <QString>
#include <QRect>
#include <QDir>
#include <QFile>
#include <QVariant>
#include <QDateTime>
#include <QElapsedTimer>
#include <QRegularExpression>

// rapidjson
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>

// fmt
#define FMT_HEADER_ONLY 1
#include <fmt/core.h>

// global includes
#include "Types.h"
#include "Utils.h"
#include "FileConf.h"
#include "Notifier.h"
#include "regex/RegexBuilder.h"
