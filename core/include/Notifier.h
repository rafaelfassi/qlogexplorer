// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QObject>

// Prevents the cost of formatting a message when its level if off
#define LOG_MESSAGE(_level, _fmt, ...)                                                                                 \
    if (Notifier::isLogLevelOn(_level))                                                                                \
    Notifier::logMsg(utl::getSrcFile(__FILE__), __LINE__, _level, FORMAT(_fmt, ##__VA_ARGS__))

#define LOG_ERR(_fmt, ...) LOG_MESSAGE(tp::NotifLevel::Error, _fmt, ##__VA_ARGS__)
#define LOG_WAR(_fmt, ...) LOG_MESSAGE(tp::NotifLevel::Warning, _fmt, ##__VA_ARGS__)
#define LOG_INF(_fmt, ...) LOG_MESSAGE(tp::NotifLevel::Info, _fmt, ##__VA_ARGS__)
#define LOG_DBG(_fmt, ...) LOG_MESSAGE(tp::NotifLevel::Debug, _fmt, ##__VA_ARGS__)


class Notifier : public QObject
{
    Q_OBJECT
public:
    static Notifier *instance()
    {
        static Notifier m_instance;
        return &m_instance;
    }

    static void setLogLevel(tp::NotifLevel level) { instance()->m_logLevel = level; }

    static bool isLogLevelOn(tp::NotifLevel level) { return (level <= instance()->m_logLevel); }

    static void logMsg(const char *file, const std::uint32_t line, tp::NotifLevel level, const std::string &msg)
    {
        if (isLogLevelOn(level))
        {
            emit instance() -> logMsgRaised(file, line, level, msg);
        }
    }

    static void notifyInfo(const QString &message)
    {
        emit instance() -> notificationRaised(tp::NotifLevel::Info, message);
    }

    static void notifyWarning(const QString &message)
    {
        emit instance() -> notificationRaised(tp::NotifLevel::Warning, message);
    }

    static void notifyError(const QString &message)
    {
        emit instance() -> notificationRaised(tp::NotifLevel::Error, message);
    }

signals:
    void logMsgRaised(const char *file, const std::uint32_t line, tp::NotifLevel level, const std::string &msg);

    // The 'message' is of QString type because the caller must use QObject::tr() to allow translation.
    void notificationRaised(tp::NotifLevel level, const QString &message);

private:
    Notifier(QObject *parent = nullptr) : QObject(parent) {}
    Notifier(const Notifier &) = delete;
    Notifier &operator=(const Notifier &) = delete;

    tp::NotifLevel m_logLevel{tp::NotifLevel::Info};
};
