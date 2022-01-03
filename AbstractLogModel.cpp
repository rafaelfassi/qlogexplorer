#include "AbstractLogModel.h"
#include <QDebug>
#include <QElapsedTimer>
#include <memory>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <regex>

AbstractLogModel::AbstractLogModel(const std::string &fileName, QObject *parent)
    : QObject(parent),
      m_fileName(fileName),
      m_ifs(fileName)
{
}

AbstractLogModel::~AbstractLogModel()
{
    stopWatch();
    stopSearch();

    if (m_ifs.is_open())
    {
        m_ifs.close();
    }
}

void AbstractLogModel::loadFile()
{
    const std::lock_guard<std::mutex> lock(m_ifsMutex);
    loadChunks();
}

const std::string &AbstractLogModel::getFileName() const
{
    return m_fileName;
}

bool AbstractLogModel::getRow(std::uint64_t row, std::vector<std::string> &rowData) const
{
    const std::lock_guard<std::mutex> lock(m_ifsMutex);

    if (row < m_rowCount)
    {
        if (!m_cachedChunkRows.contains(row))
        {
            loadChunkRowsByRow(row, m_cachedChunkRows);
            if (!m_cachedChunkRows.contains(row))
            {
                qCritical() << "Row" << row << "not found in the cache";
                return false;
            }
        }

        return parseRow(m_cachedChunkRows.get(row), rowData);
    }

    return false;
}

void AbstractLogModel::startSearch(const SearchParamLst &params)
{
    stopSearch();
    m_searchParams = params;
    m_searching = true;
    m_searchThread = std::thread(&AbstractLogModel::search, this);
}

void AbstractLogModel::stopSearch()
{
    m_searching = false;
    if (m_searchThread.joinable())
    {
        m_searchThread.join();
    }
}

void AbstractLogModel::restartSearch()
{
    if (m_searching)
    {
        stopSearch();
        startSearch(m_searchParams);
    }
}

std::string toLower(const std::string &str)
{
    std::string lower;
    lower.resize(str.size());

    for (size_t i = 0; i < str.length(); i++)
    {
        lower[i] = std::tolower(str[i]);
    }

    return lower;
}

bool matchText(const std::string &text, const std::string &sub, bool machCase)
{
    if (machCase)
    {
        return (text.find(sub) != std::string::npos);
    }
    else
    {
        const auto it = std::search(
            text.begin(),
            text.end(),
            sub.begin(),
            sub.end(),
            [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
        return (it != text.end());
    }
}

bool matchRegex(const std::string &text, const std::string &pattern, bool machCase)
{
    std::smatch m;
    std::regex::flag_type opts(std::regex::ECMAScript | std::regex::nosubs);
    if (!machCase)
    {
        opts |= std::regex::icase;
    }
    std::regex rx(pattern, opts);
    return std::regex_search(text, m, rx);
}

bool match(const std::string &text, const std::string &exp, bool machCase, bool isRegex)
{
    if (isRegex)
        return matchRegex(text, exp, machCase);
    else
        return matchText(text, exp, machCase);
}

bool searchParamsInRow(const SearchParamLst &params, const std::vector<std::string> &rowData)
{
    std::uint32_t cnt(0);
    for (const auto &param : params)
    {
        if (param.column.has_value())
        {
            if (*param.column < rowData.size())
            {
                if (match(rowData[*param.column], param.exp, param.matchCase, param.isRegex))
                {
                    if (++cnt == params.size())
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
                if (match(columnData, param.exp, param.matchCase, param.isRegex))
                {
                    if (++cnt == params.size())
                        return true;
                }
            }
        }
    }

    return false;
}

void AbstractLogModel::search()
{
    qDebug() << "Starting to search";
    QElapsedTimer timer;
    timer.start();

    ChunkRows chunkRows;
    std::vector<std::string> rowData;
    std::size_t row(0);

    while (m_searching)
    {
        for (; m_searching && (row < m_rowCount); ++row)
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
                if (searchParamsInRow(m_searchParams, rowData))
                {
                    qDebug() << "Found at row" << currRow;
                    emit valueFound(currRow);
                }

                rowData.clear();
                row = currRow;

                if (!m_searching)
                {
                    break;
                }
            }
        }

        if (m_watching)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    qDebug() << "Searching finished after" << timer.elapsed() / 1000 << "seconds";
}

std::size_t AbstractLogModel::columnCount() const
{
    return m_columns.size();
}

std::size_t AbstractLogModel::rowCount() const
{
    return m_rowCount;
}

bool AbstractLogModel::isWatching() const
{
    return m_watching;
}

void AbstractLogModel::startWatch()
{
    stopWatch();
    m_watching = true;
    m_watchThread = std::thread(&AbstractLogModel::keepWatching, this);
}

void AbstractLogModel::stopWatch()
{
    m_watching = false;
    if (m_watchThread.joinable())
    {
        m_watchThread.join();
    }
}

bool AbstractLogModel::isFollowing() const
{
    return m_following;
}

void AbstractLogModel::setFollowing(bool following)
{
    if (m_following != following)
    {
        m_following = following;
    }
}

void AbstractLogModel::keepWatching()
{
    while (m_watching)
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

        if (m_watching)
        {
            std::ifstream newIfs;

            while (m_watching)
            {
                newIfs.open(m_fileName, std::ifstream::in);
                if (newIfs.good())
                {
                    break;
                }
                newIfs.close();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            if (m_watching)
            {
                const std::lock_guard<std::mutex> lock(m_ifsMutex);
                m_ifs.close();
                m_ifs = std::move(newIfs);

                m_rowCount = 0;
                m_lastParsedPos = 0;
                m_cachedChunkRows = ChunkRows();
                m_chunks.clear();
            }
        }
    }
}

WatchingResult AbstractLogModel::watchFile()
{
    if (!m_ifs.is_open())
    {
        qDebug() << "File is not opened: " << m_fileName.c_str();
        return WatchingResult::FileClosed;
    }

    while (m_watching)
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

                if (m_following || (m_lastParsedPos == 0))
                {
                    if (!m_ifs.eof())
                    {
                        mustLoadChunks = true;
                    }
                }
            }
        }

        if (mustLoadChunks)
        {
            loadChunks();
        }

        if (m_watching)
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return WatchingResult::NormalExit;
}

ssize_t AbstractLogModel::getFileSize(std::istream &is)
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

ssize_t AbstractLogModel::getFilePos(std::istream &is)
{
    return is.tellg();
}

bool AbstractLogModel::isEndOfFile(std::istream &is)
{
    return is.eof();
}

bool AbstractLogModel::moveFilePos(std::istream &is, std::size_t pos)
{
    if (is.eof())
    {
        is.clear(std::ios::eofbit);
    }
    is.seekg(pos, std::ios::beg);
    return is.good();
}

ssize_t AbstractLogModel::readFile(std::istream &is, std::string &buffer, std::size_t bytes)
{
    is.read(buffer.data(), bytes);
    return is.gcount();
}

void AbstractLogModel::loadChunks()
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
    ssize_t lastParsedPos(0);

    do
    {
        std::ifstream ifs(m_fileName);
        moveFilePos(ifs, m_lastParsedPos);
        lastParsedPos = parseChunks(ifs, chunks, m_lastParsedPos, nextRow, fileSize);
        const std::lock_guard<std::mutex> lock(m_ifsMutex);

        m_ifs = std::move(ifs);
        if (!chunks.empty())
        {
            m_chunks.reserve(m_chunks.size() + chunks.size());
            std::move(std::begin(chunks), std::end(chunks), std::back_inserter(m_chunks));
            chunks.clear();
        }
        if (lastParsedPos > m_lastParsedPos)
        {
            m_lastParsedPos = lastParsedPos;

            size_t rowCount(m_chunks.empty() ? 0 : (m_chunks.back().getLastRow() + 1));
            if (rowCount != m_rowCount)
            {
                nextRow = rowCount;
                m_rowCount = rowCount;
                emit countChanged();
            }
        }

    } while (m_watching && (lastParsedPos < fileSize));

    chunkCount = m_chunks.size() - chunkCount;
    qDebug() << chunkCount << "chunks parsed in" << timer.elapsed() / 1000 << "seconds";
}

bool AbstractLogModel::loadChunkRowsByRow(size_t row, ChunkRows &chunkRows) const
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
