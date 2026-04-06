// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "TestUtils.h"
#include <QDirIterator>

namespace tutl
{

// LogModelSearchResults ----------------------------------------------------------------------------------------------

void LogModelSearchResults::connectModel(BaseLogModel *model)
{
    m_model = model;
    connect(model, &BaseLogModel::valueFound, this, &LogModelSearchResults::addSearchResult, Qt::DirectConnection);
}

bool LogModelSearchResults::nextRow(tp::RowData &rowData)
{
    std::lock_guard lock(m_mtx);
    if (m_model.has_value() && m_pos < m_rows.size())
    {
        if ((m_model.value()->getRow(m_rows.at(m_pos), rowData) == m_rows.at(m_pos)))
        {
            ++m_pos;
            return true;
        }
    }
    return false;
}

std::size_t LogModelSearchResults::count()
{
    std::lock_guard lock(m_mtx);
    return m_rows.size();
}

void LogModelSearchResults::addSearchResult(tp::SharedSIntList rowsPtr)
{
    std::lock_guard lock(m_mtx);

    for (auto row : *rowsPtr.get())
    {
        m_rows.push_back(row);
    }

    if (m_model.has_value())
    {
        m_model.value()->resultsDigested();
    }
}

// NotificationHandler ------------------------------------------------------------------------------------------------

NotificationHandler::NotificationHandler(bool printMsg) : m_printMsg(printMsg)
{
    connect(
        Notifier::instance(),
        &Notifier::notificationRaised,
        this,
        &NotificationHandler::addNotification,
        Qt::DirectConnection);
    connect(Notifier::instance(), &Notifier::logMsgRaised, this, &NotificationHandler::addLogMsg, Qt::DirectConnection);
}

void NotificationHandler::addNotification(tp::NotifLevel level, const QString &message)
{
    std::lock_guard lock(m_mtx);

    if (m_printMsg)
    {
        TEST_OUTPUT << FORMAT(R"_([{}]: {})_", tp::toStr<tp::NotifLevel>(level), message.toStdString()) << std::endl;
    }
    else
    {
        m_notifications.push_back({level, message});
    }
}

void NotificationHandler::addLogMsg(
    const char *file,
    const std::uint32_t line,
    tp::NotifLevel level,
    const std::string &msg)
{
    std::lock_guard lock(m_mtx);

    if (m_printMsg)
    {
        TEST_OUTPUT << FORMAT(R"_([{}] {}:{}: {})_", tp::toStr<tp::NotifLevel>(level), file, line, msg) << std::endl;
    }
    else
    {
        m_logMsgs.push_back({level, msg});
    }
}

std::vector<std::pair<tp::NotifLevel, QString>> NotificationHandler::getNotifications()
{
    std::lock_guard lock(m_mtx);
    return m_notifications;
}

std::vector<std::pair<tp::NotifLevel, std::string>> NotificationHandler::getLogMsgs()
{
    std::lock_guard lock(m_mtx);
    return m_logMsgs;
}

void NotificationHandler::clear()
{
    std::lock_guard lock(m_mtx);
    m_notifications.clear();
    m_logMsgs.clear();
}

// --------------------------------------------------------------------------------------------------------------------

InFileStream::Ptr openTestFile(const std::string &fileName)
{
    EXPECT_TRUE(QFile::exists(utl::toQStr(fileName)));
    auto ifs = InFileStream::make(fileName);
    EXPECT_TRUE(ifs);
    EXPECT_TRUE(ifs->isOpen());
    EXPECT_TRUE(ifs->getStream().good());
    return ifs;
}

void copyResourcesToCurrentDir()
{
    QDirIterator it(":/resources", QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString resPath = it.next();
        QString destPath = resPath;
        destPath.replace(":/resources/", "");

        QFileInfo info(resPath);
        if (info.isFile())
        {
            if (QFile::exists(destPath))
            {
                QFile::remove(destPath);
            }
            QFile::copy(resPath, destPath);
        }
        else if (info.isDir())
        {
            QDir().mkpath(destPath);
        }
    }
}

} // namespace tutl