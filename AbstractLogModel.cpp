#include "AbstractLogModel.h"
#include <QDebug>
#include <QElapsedTimer>
#include <memory>
#include <sstream>
#include <filesystem>

AbstractLogModel::AbstractLogModel(const std::string &fileName, QObject *parent)
    : QObject(parent),
      m_fileName(fileName),
      m_ifs(fileName)
{
}

AbstractLogModel::~AbstractLogModel()
{
    if (m_watching)
    {
        stopWatch();
    }

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
        if (loadChunkRowsByRow(row))
        {
            if (m_cachedChunkRows.contains(row))
            {
                return parseRow(m_cachedChunkRows.get(row), rowData);
            }
            qCritical() << "Row" << row << "not found in the cache";
        }
    }

    return false;
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
    if (!m_watching)
    {
        m_watching = true;
        m_watchThread = std::thread(&AbstractLogModel::keepWatching, this);
    }
}

void AbstractLogModel::stopWatch()
{
    if (m_watching)
    {
        m_watching = false;
        if (m_watchThread.joinable())
        {
            m_watchThread.join();
        }
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
                return;
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
                        loadChunks();
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return WatchingResult::NormalExit;
}

std::istream &AbstractLogModel::getFileStream() const
{
    return m_ifs;
}

std::size_t AbstractLogModel::getFilePos() const
{
    return m_ifs.tellg();
}

bool AbstractLogModel::isEndOfFile() const
{
    return m_ifs.eof();
}

bool AbstractLogModel::moveFilePos(std::size_t pos) const
{
    if (m_ifs.eof())
    {
        m_ifs.clear(std::ios::eofbit);
    }
    m_ifs.seekg(pos, std::ios::beg);
    return m_ifs.good();
}

ssize_t AbstractLogModel::readFile(std::string &buffer, std::size_t bytes)
{
    m_ifs.read(buffer.data(), bytes);
    return m_ifs.gcount();
}

void AbstractLogModel::loadChunks()
{
    qDebug() << "Starting to parse chunks for" << m_fileName.c_str();
    QElapsedTimer timer;
    timer.start();
    auto chunkCount = m_chunks.size();

    if (!m_ifs.good())
    {
        qCritical() << "The ifstream is not good";
        return;
    }

    m_ifs.seekg(0, std::ios::end);
    const size_t fileSize = m_ifs.tellg();
    m_ifs.seekg(m_lastParsedPos, std::ios::beg);

    auto lastParsedPos = parseChunks(m_lastParsedPos, fileSize);
    if (lastParsedPos < fileSize)
    {
        qWarning() << "Expecting lastParsedPos" << lastParsedPos << "to be >= fileSize" << fileSize;
        lastParsedPos = fileSize;
    }

    if (lastParsedPos > m_lastParsedPos)
    {
        m_lastParsedPos = lastParsedPos;

        size_t rowCount(m_chunks.empty() ? 0 : (m_chunks.back().getLastRow() + 1));
        if (rowCount != m_rowCount)
        {
            m_rowCount = rowCount;
            emit countChanged();
        }
    }

    chunkCount = m_chunks.size() - chunkCount;
    qDebug() << chunkCount << "chunks parsed in" << timer.elapsed() / 1000 << "seconds";
}

bool AbstractLogModel::loadChunkRowsByRow(size_t row) const
{
    if (m_cachedChunkRows.contains(row))
    {
        return true;
    }

    // qDebug() << "Loading chunk";

    for (const auto &chunk : m_chunks)
    {
        if (row >= chunk.getFistRow() && row <= chunk.getLastRow())
        {
            ChunkRows chunkRows(chunk);
            loadChunkRows(chunkRows);
            if (chunkRows.rowCount() != chunk.getRowCount())
            {
                qCritical() << "The cached chunk rows" << chunkRows.rowCount() << "does not match the chunk info"
                            << chunk.getRowCount();
            }
            m_cachedChunkRows = std::move(chunkRows);
            // qDebug() << "Chunk loaded";
            return true;
        }
    }

    qCritical() << "Cannot find a chunk containing the row" << row;
    return false;
}
