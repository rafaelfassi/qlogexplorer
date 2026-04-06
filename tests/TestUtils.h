// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "gtest/gtest.h"
#include "InFileStream.h"
#include "model/BaseLogModel.h"
#include <iostream>

// Change this to 'std::cerr' if the messages are not printed in the test output
#define TEST_OUTPUT std::cout

namespace tutl
{

class LogModelSearchResults : public QObject
{
public:
    LogModelSearchResults() {}
    LogModelSearchResults(BaseLogModel &model) { connectModel(&model); }
    LogModelSearchResults(BaseLogModel *model) { connectModel(model); }

    void connectModel(BaseLogModel *model);
    bool nextRow(tp::RowData &rowData);
    std::size_t count();

public slots:
    void addSearchResult(tp::SharedSIntList rowsPtr);

private:
    std::size_t m_pos = 0;
    tp::SIntList m_rows;
    std::mutex m_mtx; // The connection is of type Qt::DirectConnection
    std::optional<BaseLogModel *> m_model;
};

class NotificationHandler : public QObject
{
public:
    NotificationHandler(bool printMsg = false);
    std::vector<std::pair<tp::NotifLevel, QString>> getNotifications();
    std::vector<std::pair<tp::NotifLevel, std::string>> getLogMsgs();
    void clear();

public slots:
    void addNotification(tp::NotifLevel level, const QString &message);
    void addLogMsg(const char *file, const std::uint32_t line, tp::NotifLevel level, const std::string &msg);

private:
    bool m_printMsg;
    std::mutex m_mtx; // The connection is of type Qt::DirectConnection
    std::vector<std::pair<tp::NotifLevel, QString>> m_notifications;
    std::vector<std::pair<tp::NotifLevel, std::string>> m_logMsgs;
};

InFileStream::Ptr openTestFile(const std::string &fileName);
void copyResourcesToCurrentDir();

} // namespace tutl
