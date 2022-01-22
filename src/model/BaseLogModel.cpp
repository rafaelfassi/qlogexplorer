#include "pch.h"
#include "BaseLogModel.h"
#include <QElapsedTimer>
#include <QRegularExpression>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <regex>

class BaseParamMatcher
{
public:
    BaseParamMatcher(const SearchParam &param) : m_param(param) {}
    virtual bool match(const std::string &text) = 0;
    bool isRegex() const { return m_param.isRegex; }
    bool matchCase() const { return m_param.matchCase; }
    bool notOp() const { return m_param.notOp; }
    bool hasColumn() const { return m_param.column.has_value(); }
    size_t getColumn() const { return *m_param.column; }

protected:
    const SearchParam &m_param;
};

// Very slow compared to QRegularExpression that uses JIT.
// class RegexParamMatcher : public BaseParamMatcher
// {
// public:
//     RegexParamMatcher(const SearchParam &param) : BaseParamMatcher(param), m_rx(param.exp, getOpts())
//     {
//         std::regex::flag_type opts(std::regex::ECMAScript | std::regex::nosubs);
//         if (!param.matchCase)
//         {
//             opts |= std::regex::icase;
//         }
//     }

//     std::regex::flag_type getOpts()
//     {
//         std::regex::flag_type opts(std::regex::ECMAScript | std::regex::nosubs);
//         if (!m_param.matchCase)
//         {
//             opts |= std::regex::icase;
//         }
//         return opts;
//     }

//     bool match(const std::string &text) override
//     {
//         return std::regex_search(text, m_match, m_rx);
//     }

// private:
//     const std::regex m_rx;
//     std::smatch m_match;
// };

class RegexParamMatcher : public BaseParamMatcher
{
public:
    RegexParamMatcher(const SearchParam &param) : BaseParamMatcher(param), m_rx(param.exp.c_str(), getOpts()) {}

    QRegularExpression::PatternOptions getOpts()
    {
        QRegularExpression::PatternOptions opts = QRegularExpression::DontCaptureOption;
        if (!m_param.matchCase)
        {
            opts |= QRegularExpression::CaseInsensitiveOption;
        }
        return opts;
    }

    bool match(const std::string &text) override { return m_rx.match(text.c_str()).hasMatch(); }

private:
    const QRegularExpression m_rx;
};

class TextParamMatcher : public BaseParamMatcher
{
public:
    TextParamMatcher(const SearchParam &param)
        : BaseParamMatcher(param),
          m_textToSearch(param.matchCase ? m_param.exp : capitalize(m_param.exp))
    {
    }

    std::string capitalize(const std::string &text)
    {
        std::string res(text);
        std::transform(text.begin(), text.end(), res.begin(), [](int c) { return std::toupper(c); });
        return res;
    }

    // Way more farter, but works only for ascii characters.
    // std::string capitalize(const std::string &text)
    // {
    //     std::string res(text);
    //     for (auto it = res.begin(); it != res.end(); ++it)
    //         if (*it >= 'a' && *it <= 'z')
    //             *it &= ~0x20;
    //     return res;
    // }

    bool match(const std::string &text) override
    {
        if (m_param.matchCase)
            return (text.find(m_textToSearch) != std::string::npos);
        else
            return (capitalize(text).find(m_textToSearch) != std::string::npos);
    }

private:
    const std::string m_textToSearch;
};

using ParamMatchers = std::vector<std::unique_ptr<BaseParamMatcher>>;

ParamMatchers makeParamMatchers(const SearchParamLst &params)
{
    ParamMatchers matchers;
    matchers.reserve(params.size());

    for (const auto &param : params)
    {
        if (param.isRegex)
        {
            matchers.emplace_back(std::make_unique<RegexParamMatcher>(param));
        }
        else
        {
            matchers.emplace_back(std::make_unique<TextParamMatcher>(param));
        }
    }
    return matchers;
}

bool matchParamsInRow(const ParamMatchers &matchers, bool orOp, const std::vector<std::string> &rowData)
{
    std::uint32_t cnt(0);

    for (const auto &matcher : matchers)
    {
        if (matcher->hasColumn())
        {
            if (matcher->getColumn() < rowData.size())
            {
                bool matched = matcher->match(rowData[matcher->getColumn()]);
                if (matcher->notOp())
                    matched = !matched;
                if (matched)
                {
                    if ((++cnt == matchers.size()) || orOp)
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
            bool matched(false);
            for (const auto &columnData : rowData)
            {
                if (matcher->match(columnData))
                {
                    matched = true;
                    break;
                }
            }
            if (matcher->notOp())
                matched = !matched;
            if (matched)
            {
                if ((++cnt == matchers.size()) || orOp)
                    return true;
            }
        }
    }

    return false;
}

BaseLogModel::BaseLogModel(Conf &conf, QObject *parent)
    : AbstractModel(parent),
      m_conf(conf),
      m_fileName(conf.getFileName()),
      m_ifs(m_fileName)
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

tp::Columns &BaseLogModel::getColumns()
{
    return m_conf.getColumns();
}

const tp::Columns &BaseLogModel::getColumns() const
{
    return m_conf.getColumns();
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

void BaseLogModel::search()
{
    qDebug() << "Starting to search";

    ChunkRows chunkRows;
    std::vector<std::string> rowData;
    std::size_t row(0);
    const auto &matchers = makeParamMatchers(m_searchParams);

    while (m_searching.load())
    {
        QElapsedTimer timer;
        timer.start();
        qint64 searchTime(0);

        std::size_t startingRow(row);

        std::shared_ptr<std::deque<ssize_t>> rowsPtr = std::make_shared<std::deque<ssize_t>>();

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
                if (matchParamsInRow(matchers, m_searchWithOrOperator, rowData))
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
                rowsPtr = std::make_shared<std::deque<ssize_t>>();
                searchTime += timer.restart();
            }
        }

        if (!rowsPtr->empty())
        {
            emit valueFound(rowsPtr);
        }

        if (row > startingRow)
        {
            qDebug() << "Searching finished after" << searchTime / 1000 << "seconds";
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

std::size_t BaseLogModel::columnCount() const
{
    return m_conf.getColumns().size();
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
