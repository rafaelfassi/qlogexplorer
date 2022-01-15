#include "BaseLogModel.h"
#include <QDebug>
#include <QElapsedTimer>
#include <memory>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <regex>

BaseLogModel::BaseLogModel(const std::string &fileName, QObject *parent)
    : AbstractModel(parent),
      m_fileName(fileName),
      m_ifs(fileName)
{
}

BaseLogModel::~BaseLogModel()
{
    if (m_watching.load())
    {
        qCritical() << "stop() must be called on the derivated class destructor";
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

ssize_t BaseLogModel::getRow(std::uint64_t row, std::vector<std::string> &rowData) const
{
    const std::lock_guard<std::mutex> lock(m_ifsMutex);

    if (row < m_rowCount.load())
    {
        if (!m_cachedChunkRows.contains(row))
        {
            loadChunkRowsByRow(row, m_cachedChunkRows);
            if (!m_cachedChunkRows.contains(row))
            {
                qCritical() << "Row" << row << "not found in the cache";
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

const std::vector<std::string> &BaseLogModel::getColumns() const
{
    return m_columns;
}

void BaseLogModel::startSearch(const SearchParamLst &params, bool orOp)
{
    stopSearch();
    m_searchParams = params;
    m_searchWithOrOperator = orOp;
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

bool matchParam(const std::string &text, const SearchParam &param)
{
    if (param.isRegex)
    {
        std::smatch m;
        std::regex::flag_type opts(std::regex::ECMAScript | std::regex::nosubs);
        if (!param.matchCase)
        {
            opts |= std::regex::icase;
        }

        std::regex rx(param.exp, opts);

        if (param.wholeText)
        {
            return std::regex_match(text, m, rx);
        }
        else
        {
            return std::regex_search(text, m, rx);
        }
    }
    else
    {
        if (param.wholeText && (text.size() != param.exp.size()))
        {
            return false;
        }

        if (param.matchCase)
        {
            if (param.wholeText)
            {
                return (text == param.exp);
            }
            else
            {
                return (text.find(param.exp) != std::string::npos);
            }
        }
        else
        {
            const auto it = std::search(
                text.begin(),
                text.end(),
                param.exp.begin(),
                param.exp.end(),
                [](const char c1, const char c2) { return std::tolower(c1) == std::tolower(c2); });

            if (param.wholeText)
            {
                return (it == text.begin());
            }
            else
            {
                return (it != text.end());
            }
        }
    }
}

bool searchParamsInRow(const SearchParamLst &params, bool orOp, const std::vector<std::string> &rowData)
{
    std::uint32_t cnt(0);
    for (const auto &param : params)
    {
        if (param.column.has_value())
        {
            if (*param.column < rowData.size())
            {
                if (matchParam(rowData[*param.column], param))
                {
                    if ((++cnt == params.size()) || orOp)
                        return true;
                }
            }
            else
            {
                qCritical() << "Filter column is bigger than row columns";
            }
        }
        else
        {
            for (const auto &columnData : rowData)
            {
                if (matchParam(columnData, param))
                {
                    if ((++cnt == params.size()) || orOp)
                        return true;
                }
            }
        }
    }

    return false;
}

void BaseLogModel::search()
{
    qDebug() << "Starting to search";

    ChunkRows chunkRows;
    std::vector<std::string> rowData;
    std::size_t row(0);

    while (m_searching.load())
    {
        QElapsedTimer timer;
        timer.start();

        std::size_t startingRow(row);

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
                if (searchParamsInRow(m_searchParams, m_searchWithOrOperator, rowData))
                {
                    emit valueFound(currRow);
                }

                rowData.clear();
                row = currRow + 1;

                if (!m_searching.load(std::memory_order_relaxed))
                {
                    break;
                }
            }
        }

        if (row > startingRow)
        {
            qDebug() << "Searching finished after" << timer.elapsed() / 1000 << "seconds";
        }

        if (m_searching.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

void BaseLogModel::tryConfigure()
{
    if (m_columns.empty())
    {
        m_ifs.clear();
        m_ifs.seekg(0, std::ios::beg);
        configure(m_ifs);
        if (!m_columns.empty())
            emit modelConfigured();
    }
}

std::size_t BaseLogModel::columnCount() const
{
    return m_columns.size();
}

std::size_t BaseLogModel::rowCount() const
{
    return m_rowCount.load();
}

ssize_t BaseLogModel::getRowNum(ssize_t row) const
{
    return row;
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
                qCritical() << "Unknown WatchingResult";
                break;
        }

        if (m_watching.load())
        {
            std::ifstream newIfs;

            do
            {
                newIfs.open(m_fileName, std::ifstream::in);
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

                m_rowCount.store(0);
                m_lastParsedPos = 0;
                m_cachedChunkRows = ChunkRows();
                m_chunks.clear();
            }
        }
    }
}

WatchingResult BaseLogModel::watchFile()
{
    if (!m_ifs.is_open())
    {
        qDebug() << "File is not opened: " << m_fileName.c_str();
        return WatchingResult::FileClosed;
    }

    while (m_watching.load())
    {
        bool mustLoadChunks(false);

        if (!std::filesystem::exists(m_fileName))
        {
            qDebug() << "File does not exist: " << m_fileName.c_str();
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
                    qDebug() << "File was recreated: " << m_fileName.c_str();
                    return WatchingResult::FileRecreated;
                }

                m_ifs.seekg(m_lastParsedPos, std::ios::beg);
                m_ifs.peek();
                if (m_ifs.fail())
                {
                    qDebug() << "File is on fail status: " << m_fileName.c_str();
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

void BaseLogModel::addColumn(const std::string &name)
{
    m_columns.push_back(name);
}

ssize_t BaseLogModel::getFileSize(std::istream &is)
{
    ssize_t fileSize(0);
    ssize_t oriPos(0);
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

ssize_t BaseLogModel::getFilePos(std::istream &is)
{
    return is.tellg();
}

bool BaseLogModel::isEndOfFile(std::istream &is)
{
    return is.eof();
}

bool BaseLogModel::moveFilePos(std::istream &is, std::size_t pos)
{
    if (is.eof())
    {
        is.clear(std::ios::eofbit);
    }
    is.seekg(pos, std::ios::beg);
    return is.good();
}

ssize_t BaseLogModel::readFile(std::istream &is, std::string &buffer, std::size_t bytes)
{
    is.read(buffer.data(), bytes);
    return is.gcount();
}

void BaseLogModel::loadChunks()
{
    qDebug() << "Starting to parse chunks for" << m_fileName.c_str();
    QElapsedTimer timer;
    timer.start();

    ssize_t fileSize(0);
    size_t nextRow(0);
    size_t chunkCount(0);

    {
        const std::lock_guard<std::mutex> lock(m_ifsMutex);
        if (!m_ifs.good())
        {
            qCritical() << "The ifstream is not good";
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
    ssize_t newLastParsedPos(0);

    do
    {
        std::ifstream ifs(m_fileName);
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

            size_t rowCount(m_chunks.empty() ? 0 : (m_chunks.back().getLastRow() + 1));
            if (rowCount != m_rowCount.load())
            {
                nextRow = rowCount;
                m_rowCount.store(rowCount);
                emit countChanged();
            }
        }

    } while (m_watching.load(std::memory_order_relaxed) && (newLastParsedPos < fileSize));

    chunkCount = m_chunks.size() - chunkCount;
    qDebug() << chunkCount << "chunks parsed in" << timer.elapsed() / 1000 << "seconds";
}

bool BaseLogModel::loadChunkRowsByRow(size_t row, ChunkRows &chunkRows) const
{
    const auto chunk = std::lower_bound(m_chunks.begin(), m_chunks.end(), row, Chunk::compareRows);
    if ((chunk != m_chunks.end()) && chunk->countainRow(row))
    {
        ChunkRows tmpChunkRows(*chunk);
        loadChunkRows(m_ifs, tmpChunkRows);
        if (tmpChunkRows.rowCount() != chunk->getRowCount())
        {
            qCritical() << "The cached chunk rows" << tmpChunkRows.rowCount() << "does not match the chunk info"
                        << chunk->getRowCount();
        }
        chunkRows = std::move(tmpChunkRows);
        return true;
    }
    return false;
}
