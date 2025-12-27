// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "AbstractModel.h"
#include "InFileStream.h"
#include "Matcher.h"
#include <thread>
#include <mutex>

constexpr tp::UInt g_chunkSize = (1024 * 1024) * 10;

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
    Chunk(tp::UInt startPos, tp::UInt endPos, tp::UInt firstRow, tp::UInt lastRow)
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
    std::pair<tp::UInt, tp::UInt> m_posRange;
    std::pair<tp::UInt, tp::UInt> m_rowRange;
};

class ChunkRows
{
public:
    using ChunkRowsData = std::pair<tp::UInt, std::string>;

    ChunkRows(const Chunk &chunk) : m_chunk(&chunk) {}
    ChunkRows() = default;
    void add(tp::UInt row, const std::string &content) { m_rows.emplace_back(row, content); }
    void reserve(tp::UInt size) { m_rows.reserve(size); }
    const std::string &get(tp::UInt row) const
    {
        const auto it = std::lower_bound(m_rows.begin(), m_rows.end(), row, compareRows);
        if ((it != m_rows.end()) && (row == it->first))
            return it->second;
        throw std::runtime_error("Row not found");
    }
    bool contains(tp::UInt row) const
    {
        const auto it = std::lower_bound(m_rows.begin(), m_rows.end(), row, compareRows);
        return (it != m_rows.end() && row == it->first);
    }
    tp::UInt rowCount() const { return m_rows.size(); }
    const Chunk *getChunk() { return m_chunk; }
    const std::vector<ChunkRowsData> &data() { return m_rows; }

    static bool compareRows(const ChunkRowsData &rowData, const tp::UInt &row) { return (rowData.first < row); }

private:
    const Chunk *m_chunk = nullptr;
    std::vector<ChunkRowsData> m_rows;
};

class BaseLogModel : public AbstractModel
{
    Q_OBJECT

public:
    BaseLogModel(FileConf::Ptr conf, QObject *parent = 0);
    virtual ~BaseLogModel();
    const std::string &getFileName() const;
    tp::SInt getRow(tp::SInt row, tp::RowData &rowData) const override final;
    bool hasDefinedColumns() const override final;
    tp::Columns &getColumns() override final;
    const tp::Columns &getColumns() const override final;
    tp::UInt columnCount() const override final;
    tp::UInt rowCount() const override final;
    tp::SInt getRowNum(tp::SInt row) const override final;
    tp::SInt getNoMatchColumn() const;
    void startSearch(const tp::SearchParams &params, bool orOp);
    void stopSearch();
    bool isSearching() const;
    bool isWatching() const;
    void start();
    void stop();
    void reconfigure();
    bool isFollowing() const;

signals:
    void parsingProgressChanged(int progress);
    void searchingProgressChanged(int progress);
    void valueFound(tp::SharedSIntList rowsPtr) const;

public slots:
    void setFollowing(bool following);

protected:
    virtual bool configure(FileConf::Ptr conf, std::istream &is) = 0;
    virtual bool parseRow(const std::string &rawText, tp::RowData &rowData) const = 0;
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
    bool loadChunkRowsByRow(tp::UInt row, ChunkRows &chunkRows) const;
    void keepWatching();
    WatchingResult watchFile();
    void search();
    void tryConfigure();
    FileConf::Ptr m_conf;
    std::string m_fileName;
    mutable InFileStream::Ptr m_ifs;
    mutable std::mutex m_ifsMutex;
    mutable ChunkRows m_cachedChunkRows;
    std::vector<Chunk> m_chunks;
    Matcher m_matcher;
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
