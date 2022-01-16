#pragma once

#include "AbstractModel.h"
#include <fstream>
#include <thread>
#include <mutex>

constexpr size_t g_chunkSize = 1024 * 1024;

enum class WatchingResult
{
    NormalExit,
    FileClosed,
    FileNotFound,
    FileRecreated,
    UnknownFailure
};

class Chunk
{
public:
    Chunk(size_t startPos, size_t endPos, size_t firstRow, size_t lastRow)
        : m_posRange(std::make_pair(startPos, endPos)),
          m_rowRange(std::make_pair(firstRow, lastRow))
    {
    }
    size_t getSize() const { return m_posRange.second - m_posRange.first + 1; }
    size_t getStartPos() const { return m_posRange.first; }
    size_t getEndPos() const { return m_posRange.second; }

    size_t getRowCount() const { return m_rowRange.second - m_rowRange.first + 1; }
    size_t getFistRow() const { return m_rowRange.first; }
    size_t getLastRow() const { return m_rowRange.second; }

    bool countainRow(const size_t &row) const { return ((row >= getFistRow()) && (row <= getLastRow())); }
    static bool compareRows(const Chunk &c, const size_t &row) { return (c.getLastRow() < row); }

private:
    std::pair<size_t, size_t> m_posRange;
    std::pair<size_t, size_t> m_rowRange;
};

class ChunkRows
{
public:
    using RowsData = std::pair<size_t, std::string>;

    ChunkRows(const Chunk &chunk) : m_chunk(&chunk) {}
    ChunkRows() = default;
    void add(size_t row, const std::string &content) { m_rows.emplace_back(row, content); }
    void reserve(size_t size) { m_rows.reserve(size); }
    const std::string &get(size_t row) const
    {
        const auto it = std::lower_bound(m_rows.begin(), m_rows.end(), row, compareRows);
        if ((it != m_rows.end()) && (row == it->first))
            return it->second;
        throw std::runtime_error("Row not found");
    }
    bool contains(size_t row) const
    {
        const auto it = std::lower_bound(m_rows.begin(), m_rows.end(), row, compareRows);
        return (it != m_rows.end() && row == it->first);
    }
    size_t rowCount() const { return m_rows.size(); }
    const Chunk *getChunk() { return m_chunk; }
    const std::vector<RowsData> &data() { return m_rows; }

    static bool compareRows(const RowsData &rowData, const size_t &row) { return (rowData.first < row); }

private:
    const Chunk *m_chunk = nullptr;
    std::vector<RowsData> m_rows;
};

struct SearchParam
{
    bool isRegex = false;
    bool matchCase = true;
    bool wholeText = false;
    bool notOp = false;
    std::string exp;
    std::optional<std::size_t> column;
};
using SearchParamLst = std::vector<SearchParam>;

class BaseLogModel : public AbstractModel
{
    Q_OBJECT

public:
    BaseLogModel(const std::string &fileName, QObject *parent = 0);
    virtual ~BaseLogModel();
    const std::string &getFileName() const;
    ssize_t getRow(std::uint64_t row, std::vector<std::string> &rowData) const override final;
    const std::vector<std::string> &getColumns() const override final;
    std::size_t columnCount() const override final;
    std::size_t rowCount() const override final;
    ssize_t getRowNum(ssize_t row) const override final;
    void startSearch(const SearchParamLst &params, bool orOp);
    void stopSearch();
    bool isSearching() const;
    bool isWatching() const;
    void start();
    void stop();
    bool isFollowing() const;

signals:
    void parsingProgress(char progress);
    void valueFound(std::shared_ptr<std::deque<ssize_t>> rowsPtr) const;

public slots:
    void setFollowing(bool following);

protected:
    virtual void configure(std::istream &is) = 0;
    virtual bool parseRow(const std::string &rawText, std::vector<std::string> &rowData) const = 0;
    virtual std::size_t parseChunks(
        std::istream &is,
        std::vector<Chunk> &chunks,
        std::size_t fromPos,
        std::size_t nextRow,
        std::size_t fileSize) = 0;
    virtual void loadChunkRows(std::istream &is, ChunkRows &chunkRows) const = 0;

    void addColumn(const std::string &name);

    // Helping funtions to operate over istream.
    static ssize_t getFileSize(std::istream &is);
    static ssize_t getFilePos(std::istream &is);
    static bool isEndOfFile(std::istream &is);
    static bool moveFilePos(std::istream &is, std::size_t pos);
    static ssize_t readFile(std::istream &is, std::string &buffer, std::size_t bytes);

private:
    void loadChunks();
    bool loadChunkRowsByRow(size_t row, ChunkRows &chunkRows) const;
    void keepWatching();
    WatchingResult watchFile();
    void search();
    void tryConfigure();
    std::string m_fileName;
    mutable std::ifstream m_ifs;
    mutable std::mutex m_ifsMutex;
    mutable ChunkRows m_cachedChunkRows;
    std::vector<std::string> m_columns;
    std::vector<Chunk> m_chunks;
    SearchParamLst m_searchParams;
    bool m_searchWithOrOperator;
    std::thread m_searchThread;
    std::thread m_watchThread;
    // Control flags that are set in the main thread and read by other threads.
    std::atomic_bool m_searching = false;
    std::atomic_bool m_watching = false;
    std::atomic_bool m_following = true;
    // Set by m_watchThread and read by main and m_searchThread threads.
    std::atomic_size_t m_rowCount = 0;
    // Accessed only by m_watchThread.
    std::size_t m_lastParsedPos = 0;
};
