// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "BaseLogModel.h"

BaseLogModel::BaseLogModel(FileConf::Ptr conf, QObject *parent)
    : AbstractModel(parent),
      m_conf(conf),
      m_fileName(conf->getFileName()),
      m_ifs(m_fileName, std::ifstream::in | std::ifstream::binary)
{
}

BaseLogModel::~BaseLogModel()
{
    if (m_watching.load())
    {
        LOG_ERR("stop() must be called on the derivated class destructor");
        stop();
    }

    if (m_ifs.is_open())
    {
        m_ifs.close();
    }
}

const std::string &BaseLogModel::getFileName() const
{
    return m_fileName;
}

tp::SInt BaseLogModel::getRow(tp::SInt row, tp::RowData &rowData) const
{
    const std::lock_guard<std::mutex> lock(m_ifsMutex);

    if ((-1L < row) && (row < m_rowCount.load()))
    {
        if (!m_cachedChunkRows.contains(row))
        {
            loadChunkRowsByRow(row, m_cachedChunkRows);
            if (!m_cachedChunkRows.contains(row))
            {
                LOG_ERR("Row {} not found in the cache", row);
                return -1;
            }
        }

        if (parseRow(m_cachedChunkRows.get(row), rowData))
        {
            return row;
        }
    }

    return -1;
}

tp::Columns &BaseLogModel::getColumns()
{
    return m_conf->getColumns();
}

const tp::Columns &BaseLogModel::getColumns() const
{
    return m_conf->getColumns();
}

void BaseLogModel::startSearch(const tp::SearchParams &params, bool orOp)
{
    stopSearch();
    m_matcher.setParams(params, orOp);
    m_searching.store(true);
    m_searchThread = std::thread(&BaseLogModel::search, this);
}

void BaseLogModel::stopSearch()
{
    m_searching.store(false);
    if (m_searchThread.joinable())
    {
        m_searchThread.join();
    }
}

bool BaseLogModel::isSearching() const
{
    return m_searching.load();
}

void BaseLogModel::search()
{
    LOG_INF("Starting to search");

    ChunkRows chunkRows;
    tp::RowData rowData;
    tp::UInt row(0);

    while (m_searching.load())
    {
        QElapsedTimer timer;
        timer.start();
        qint64 searchTime(0);

        tp::UInt startingRow(row);

        tp::SharedSIntList rowsPtr = std::make_shared<tp::SIntList>();

        while ((row < m_rowCount.load()) && m_searching.load(std::memory_order_relaxed))
        {
            {
                const std::lock_guard<std::mutex> lock(m_ifsMutex);
                if (!loadChunkRowsByRow(row, chunkRows))
                {
                    break;
                }
            }

            for (const auto &[currRow, rawText] : chunkRows.data())
            {
                parseRow(rawText, rowData);
                if (m_matcher.matchInRow(rowData))
                {
                    rowsPtr->push_back(currRow);
                }

                rowData.clear();
                row = currRow + 1;

                if (!m_searching.load(std::memory_order_relaxed))
                {
                    break;
                }
            }

            if (!rowsPtr->empty() && timer.hasExpired(1000))
            {
                emit valueFound(rowsPtr);
                rowsPtr = std::make_shared<tp::SIntList>();
                searchTime += timer.restart();
            }
        }

        if (!rowsPtr->empty())
        {
            emit valueFound(rowsPtr);
        }

        if (row > startingRow)
        {
            LOG_INF("Searching finished after {} seconds", searchTime / 1000);
        }

        if (m_searching.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

void BaseLogModel::tryConfigure()
{
    if (!m_configured.load())
    {
        m_ifs.clear();
        m_ifs.seekg(0, std::ios::beg);
        if (configure(m_conf, m_ifs))
        {
            m_configured.store(true);
            emit modelConfigured();
        }
    }
}

tp::UInt BaseLogModel::columnCount() const
{
    return m_conf->getColumns().size();
}

tp::UInt BaseLogModel::rowCount() const
{
    return m_rowCount.load();
}

tp::SInt BaseLogModel::getRowNum(tp::SInt row) const
{
    return row;
}

tp::SInt BaseLogModel::getNoMatchColumn() const
{
    return m_conf->getNoMatchColumn();
}

bool BaseLogModel::isWatching() const
{
    return m_watching.load();
}

void BaseLogModel::start()
{
    stop();
    tryConfigure();
    m_watching.store(true);
    m_watchThread = std::thread(&BaseLogModel::keepWatching, this);
}

void BaseLogModel::stop()
{
    m_watching.store(false);
    if (m_watchThread.joinable())
    {
        m_watchThread.join();
    }
    stopSearch();
}

void BaseLogModel::reconfigure()
{
    stop();
    m_configured.store(false);
    start();
}

void BaseLogModel::clear()
{
    m_rowCount.store(0);
    m_lastParsedPos = 0;
    m_cachedChunkRows = ChunkRows();
    m_chunks.clear();
}

bool BaseLogModel::isFollowing() const
{
    return m_following.load();
}

void BaseLogModel::setFollowing(bool following)
{
    m_following.store(following);
}

void BaseLogModel::keepWatching()
{
    while (m_watching.load())
    {
        const auto result = watchFile();

        switch (result)
        {
            case WatchingResult::NormalExit:
                return;
            case WatchingResult::FileNotFound:
            case WatchingResult::FileClosed:
            case WatchingResult::FileRecreated:
            case WatchingResult::UnknownFailure:
                break;
            default:
                LOG_ERR("Unknown WatchingResult");
                break;
        }

        if (m_watching.load())
        {
            std::ifstream newIfs;

            do
            {
                newIfs.open(m_fileName, std::ifstream::in | std::ifstream::binary);
                if (newIfs.good())
                {
                    break;
                }
                newIfs.close();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            } while (m_watching.load(std::memory_order_relaxed));

            if (m_watching.load())
            {
                const std::lock_guard<std::mutex> lock(m_ifsMutex);
                m_ifs.close();
                m_ifs = std::move(newIfs);
                clear();
            }
        }
    }
}

WatchingResult BaseLogModel::watchFile()
{
    if (!m_ifs.is_open())
    {
        LOG_WAR("File '{}' is not opened", m_fileName);
        return WatchingResult::FileClosed;
    }

    while (m_watching.load())
    {
        bool mustLoadChunks(false);
        if (!QFile::exists(m_fileName.c_str()))
        {
            LOG_WAR("File '{}' does not exist", m_fileName);
            return WatchingResult::FileNotFound;
        }
        else
        {
            const std::lock_guard<std::mutex> lock(m_ifsMutex);

            tryConfigure();

            m_ifs.clear();
            m_ifs.seekg(0, std::ios::end);
            auto fileSize = m_ifs.tellg();

            if (m_lastParsedPos != fileSize)
            {
                if (fileSize < m_lastParsedPos)
                {
                    LOG_WAR("File '{}' was recreated", m_fileName);
                    return WatchingResult::FileRecreated;
                }

                m_ifs.seekg(m_lastParsedPos, std::ios::beg);
                m_ifs.peek();
                if (m_ifs.fail())
                {
                    LOG_ERR("File '{}' is on fail status", m_fileName);
                    return WatchingResult::UnknownFailure;
                }

                if ((m_lastParsedPos == 0) || m_following.load())
                {
                    if (!m_ifs.eof())
                    {
                        // mutex needs to be released belore starting to load chunks.
                        mustLoadChunks = true;
                    }
                }
            }
        }

        if (mustLoadChunks)
        {
            loadChunks();
        }

        if (m_watching.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    return WatchingResult::NormalExit;
}

tp::SInt BaseLogModel::getFileSize(std::istream &is)
{
    tp::SInt fileSize(0);
    tp::SInt oriPos(0);
    const bool eof(is.eof());

    if (eof)
    {
        is.clear(std::ios::eofbit);
    }
    else
    {
        oriPos = is.tellg();
    }

    is.seekg(0, std::ios::end);

    if (is.good())
    {
        fileSize = is.tellg();
        if (eof)
        {
            // Back to eof
            is.get();
        }
        else
        {
            is.seekg(oriPos, std::ios::beg);
        }
    }

    return fileSize < 0 ? 0 : fileSize;
}

tp::SInt BaseLogModel::getFilePos(std::istream &is)
{
    return is.tellg();
}

bool BaseLogModel::isEndOfFile(std::istream &is)
{
    return is.eof();
}

bool BaseLogModel::moveFilePos(std::istream &is, tp::UInt pos)
{
    if (is.eof())
    {
        is.clear(std::ios::eofbit);
    }
    is.seekg(pos, std::ios::beg);
    return is.good();
}

tp::SInt BaseLogModel::readFile(std::istream &is, std::string &buffer, tp::UInt bytes)
{
    is.read(buffer.data(), bytes);
    return is.gcount();
}

void BaseLogModel::loadChunks()
{
    LOG_INF("Starting to parse chunks for '{}'", m_fileName);
    QElapsedTimer timer;
    timer.start();

    tp::SInt fileSize(0);
    tp::UInt nextRow(0);
    tp::UInt chunkCount(0);

    {
        const std::lock_guard<std::mutex> lock(m_ifsMutex);
        if (!m_ifs.good())
        {
            LOG_ERR("The ifstream is not good");
            return;
        }
        fileSize = getFileSize(m_ifs);
        chunkCount = m_chunks.size();
        if (!m_chunks.empty())
        {
            nextRow = m_chunks.back().getLastRow() + 1;
        }
    }

    std::vector<Chunk> chunks;
    tp::SInt newLastParsedPos(0);

    do
    {
        std::ifstream ifs(m_fileName, std::ifstream::in | std::ifstream::binary);
        moveFilePos(ifs, m_lastParsedPos);
        newLastParsedPos = parseChunks(ifs, chunks, m_lastParsedPos, nextRow, fileSize);
        const std::lock_guard<std::mutex> lock(m_ifsMutex);

        m_ifs = std::move(ifs);
        if (!chunks.empty())
        {
            m_chunks.reserve(m_chunks.size() + chunks.size());
            std::move(std::begin(chunks), std::end(chunks), std::back_inserter(m_chunks));
            chunks.clear();
        }
        if (newLastParsedPos > m_lastParsedPos)
        {
            m_lastParsedPos = newLastParsedPos;

            tp::UInt rowCount(m_chunks.empty() ? 0 : (m_chunks.back().getLastRow() + 1));
            if (rowCount != m_rowCount.load())
            {
                nextRow = rowCount;
                m_rowCount.store(rowCount);
                emit countChanged();
            }
        }

    } while (m_watching.load(std::memory_order_relaxed) && (newLastParsedPos < fileSize));

    chunkCount = m_chunks.size() - chunkCount;
    LOG_INF("{} chunks parsed in {} seconds", chunkCount, timer.elapsed() / 1000);
}

bool BaseLogModel::loadChunkRowsByRow(tp::UInt row, ChunkRows &chunkRows) const
{
    const auto chunk = std::lower_bound(m_chunks.begin(), m_chunks.end(), row, Chunk::compareRows);
    if ((chunk != m_chunks.end()) && chunk->countainRow(row))
    {
        ChunkRows tmpChunkRows(*chunk);
        loadChunkRows(m_ifs, tmpChunkRows);
        if (tmpChunkRows.rowCount() != chunk->getRowCount())
        {
            LOG_ERR(
                "The cached chunk rows {} does not match the chunk info {}",
                tmpChunkRows.rowCount(),
                chunk->getRowCount());
        }
        chunkRows = std::move(tmpChunkRows);
        return true;
    }
    return false;
}
