#pragma once

#include <QObject>

enum class NotificationType
{
    INFO,
    WARNING,
    ERROR
};

class Notifier : public QObject
{
    Q_OBJECT
public:
    static Notifier *instance()
    {
        static Notifier m_instance;
        return &m_instance;
    }

    static void notifyInfo(const QString &message)
    {
        emit instance() -> raiseNotification(message, NotificationType::INFO);
    }

    static void notifyWarning(const QString &message)
    {
        emit instance() -> raiseNotification(message, NotificationType::WARNING);
    }

    static void notifyError(const QString &message)
    {
        emit instance() -> raiseNotification(message, NotificationType::ERROR);
    }

signals:
    void raiseNotification(const QString &message, NotificationType type);

private:
    Notifier(QObject *parent = nullptr) : QObject(parent) {}
    Notifier(const Notifier &) = delete;
    Notifier &operator=(const Notifier &) = delete;
};
