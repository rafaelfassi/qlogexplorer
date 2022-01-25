#pragma once

#include "AbstractModel.h"
#include <fstream>
#include <thread>
#include <mutex>

constexpr tp::UInt g_chunkSize = 1024 * 1024;

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
    Chunk(size_t startPos, tp::UInt endPos, tp::UInt firstRow, tp::UInt lastRow)
        : m_posRange(std::make_pair(startPos, endPos)),
          m_rowRange(std::make_pair(firstRow, lastRow))
    {
    }
    tp::UInt getSize() const { return m_posRange.second - m_posRange.first + 1; }
    tp::UInt getStartPos() const { return m_posRange.first; }
    tp::UInt getEndPos() const { return m_posRange.second; }

    tp::UInt getRowCount() const { return m_rowRange.second - m_rowRange.first + 1; }
    tp::UInt getFistRow() const { return m_rowRange.first; }
    tp::UInt getLastRow() const { return m_rowRange.second; }

    bool countainRow(const tp::UInt &row) const { return ((row >= getFistRow()) && (row <= getLastRow())); }
    static bool compareRows(const Chunk &c, const tp::UInt &row) { return (c.getLastRow() < row); }

private:
    std::pair<size_t, tp::UInt> m_posRange;
    std::pair<size_t, tp::UInt> m_rowRange;
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
    tp::UInt rowCount() const { return m_rows.size(); }
    const Chunk *getChunk() { return m_chunk; }
    const std::vector<RowsData> &data() { return m_rows; }

    static bool compareRows(const RowsData &rowData, const tp::UInt &row) { return (rowData.first < row); }

private:
    const Chunk *m_chunk = nullptr;
    std::vector<RowsData> m_rows;
};

struct SearchParam
{
    bool isRegex = false;
    bool matchCase = true;
    bool notOp = false;
    std::string exp;
    std::optional<tp::UInt> column;
};
using SearchParamLst = std::vector<SearchParam>;

class BaseLogModel : public AbstractModel
{
    Q_OBJECT

public:
    BaseLogModel(Conf &conf, QObject *parent = 0);
    virtual ~BaseLogModel();
    const std::string &getFileName() const;
    tp::SInt getRow(std::uint64_t row, std::vector<std::string> &rowData) const override final;
    tp::Columns &getColumns() override final;
    const tp::Columns &getColumns() const override final;
    tp::UInt columnCount() const override final;
    tp::UInt rowCount() const override final;
    tp::SInt getRowNum(tp::SInt row) const override final;
    void startSearch(const SearchParamLst &params, bool orOp);
    void stopSearch();
    bool isSearching() const;
    bool isWatching() const;
    void start();
    void stop();
    void reconfigure();
    bool isFollowing() const;

signals:
    void parsingProgress(char progress);
    void valueFound(tp::SharedSIntList rowsPtr) const;

public slots:
    void setFollowing(bool following);

protected:
    virtual bool configure(Conf &conf, std::istream &is) = 0;
    virtual bool parseRow(const std::string &rawText, std::vector<std::string> &rowData) const = 0;
    virtual tp::UInt parseChunks(
        std::istream &is,
        std::vector<Chunk> &chunks,
        tp::UInt fromPos,
        tp::UInt nextRow,
        tp::UInt fileSize) = 0;
    virtual void loadChunkRows(std::istream &is, ChunkRows &chunkRows) const = 0;

    // Helping funtions to operate over istream.
    static tp::SInt getFileSize(std::istream &is);
    static tp::SInt getFilePos(std::istream &is);
    static bool isEndOfFile(std::istream &is);
    static bool moveFilePos(std::istream &is, tp::UInt pos);
    static tp::SInt readFile(std::istream &is, std::string &buffer, tp::UInt bytes);

private:
    void clear();
    void loadChunks();
    bool loadChunkRowsByRow(size_t row, ChunkRows &chunkRows) const;
    void keepWatching();
    WatchingResult watchFile();
    void search();
    void tryConfigure();
    Conf &m_conf;
    std::string m_fileName;
    mutable std::ifstream m_ifs;
    mutable std::mutex m_ifsMutex;
    mutable ChunkRows m_cachedChunkRows;
    std::vector<Chunk> m_chunks;
    SearchParamLst m_searchParams;
    bool m_searchWithOrOperator;
    std::thread m_searchThread;
    std::thread m_watchThread;
    // Control flags that are set in the main thread and read by other threads.
    std::atomic_bool m_searching = false;
    std::atomic_bool m_watching = false;
    std::atomic_bool m_following = true;
    std::atomic_bool m_configured = false;
    // Set by m_watchThread and read by main and m_searchThread threads.
    std::atomic_size_t m_rowCount = 0;
    // Accessed only by m_watchThread.
    tp::UInt m_lastParsedPos = 0;
};
