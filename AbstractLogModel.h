#pragma once

#include <QObject>
#include <fstream>
#include <thread>
#include <deque>
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

private:
    std::pair<size_t, size_t> m_posRange;
    std::pair<size_t, size_t> m_rowRange;
};

class ChunkRows
{
public:
    ChunkRows(const Chunk &chunk) : m_chunk(&chunk) {}
    ChunkRows() = default;
    void add(size_t row, const std::string &content) { m_rows.emplace(row, content); }
    const std::string &get(size_t row) const { return m_rows.at(row); }
    bool contains(size_t row) const { return (m_rows.find(row) != m_rows.end()); }
    size_t rowCount() { return m_rows.size(); }
    const Chunk *getChunk() { return m_chunk; }

private:
    const Chunk *m_chunk = nullptr;
    std::map<size_t, std::string> m_rows;
};

class AbstractLogModel : public QObject
{
    Q_OBJECT

public:
    AbstractLogModel(const std::string &fileName, QObject *parent = 0);
    virtual ~AbstractLogModel();
    void loadFile();
    const std::string& getFileName() const;
    bool getRow(std::uint64_t row, std::vector<std::string> &rowData) const;
    std::size_t columnCount() const;
    std::size_t rowCount() const;
    void startWatch();
    void stopWatch();

signals:
    void countChanged();
    void parsingProgress(char progress);

protected:
    virtual bool parseRow(const std::string &rawText, std::vector<std::string> &rowData) const = 0;
    virtual std::size_t parseChunks(std::size_t fromPos, std::size_t fileSize) = 0;
    virtual void loadChunkRows(ChunkRows &chunkRows) const = 0;

    std::istream &getFileStream() const;
    std::size_t getFilePos() const;
    bool isEndOfFile() const;
    bool moveFilePos(std::size_t pos) const;
    ssize_t readFile(std::string &buffer, std::size_t bytes);
    std::vector<Chunk> m_chunks;
    std::vector<std::string> m_columns;

private:
    void loadChunks();
    bool loadChunkRowsByRow(size_t row) const;
    void keepWatching();
    WatchingResult watchFile();
    mutable std::ifstream m_ifs;
    mutable std::mutex m_ifsMutex;
    mutable ChunkRows m_cachedChunkRows;
    std::string m_fileName;
    std::thread m_watchThread;
    std::atomic_bool m_watching = false;
    std::atomic_size_t m_lastParsedPos = 0;
    std::size_t m_rowCount = 0;
};
