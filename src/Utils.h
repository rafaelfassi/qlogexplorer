#pragma once

#define LOG_INF(...) utl::log(utl::getSrcFile(__FILE__), __LINE__, tp::LogLevel::Info, fmt::format(__VA_ARGS__))
#define LOG_WAR(...) utl::log(utl::getSrcFile(__FILE__), __LINE__, tp::LogLevel::Warning, fmt::format(__VA_ARGS__))
#define LOG_ERR(...) utl::log(utl::getSrcFile(__FILE__), __LINE__, tp::LogLevel::Error, fmt::format(__VA_ARGS__))

namespace utl
{

static constexpr const char *getSrcFile(const char *const path)
{
    return path + sizeof(APP_BASE_SRC_DIR);
}

void log(const char *file, const std::uint32_t line, tp::LogLevel level, const std::string &msg);

std::string toStr(const rapidjson::Value &json);

std::string toStr(const QString &str);

std::string join(const std::vector<std::string> &strList, const std::string &delim);

std::vector<std::string> split(const std::string &str, const std::string &delim);

QString elideLeft(const std::string &str, tp::UInt maxSize);

} // namespace utl
