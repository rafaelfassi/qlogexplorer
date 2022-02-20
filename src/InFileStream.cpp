// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "InFileStream.h"

#if defined(__MINGW32__)

#include <windows.h>
#include <Io.h>
#include <fcntl.h>
#include <ext/stdio_filebuf.h>

class InFileStreamImp : public std::istream
{
public:
    InFileStreamImp(const std::string &fileName)
    {
        m_handle = CreateFile(
            fileName.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            0,
            OPEN_EXISTING,
            0,
            0);
        int file_descriptor = _open_osfhandle((intptr_t)m_handle, _O_RDONLY);
        if (file_descriptor != -1)
        {
            FILE *file = _fdopen(file_descriptor, "rb");
            int posix_handle = fileno(file);
            m_filebuf = __gnu_cxx::stdio_filebuf<char>(posix_handle, std::ios::in | std::ifstream::binary);
        }
        else if (m_handle != nullptr)
        {
            CloseHandle(m_handle);
            m_handle = nullptr;
        }
        rdbuf(&m_filebuf);
    }

    ~InFileStreamImp()
    {
        if (m_handle != nullptr)
            closeImp();
    }

    bool isOpenImp() const { return ((m_handle != nullptr) && m_filebuf.is_open()); }

    void closeImp()
    {
        if ((m_handle != nullptr))
        {
            CloseHandle(m_handle);
            m_handle = nullptr;
        }
    }

    std::istream &getStreamImp() { return *this; }

private:
    HANDLE m_handle = nullptr;
    __gnu_cxx::stdio_filebuf<char> m_filebuf;
};

#elif defined(_MSC_VER)

#include <windows.h>
#include <Io.h>
#include <fcntl.h>

class InFileStreamImp
{
public:
    InFileStreamImp(const std::string &fileName)
    {
        HANDLE handle = CreateFile(
            fileName.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            0,
            OPEN_EXISTING,
            0,
            0);
        int file_descriptor = _open_osfhandle((intptr_t)handle, _O_RDONLY);
        if (file_descriptor != -1)
        {
            FILE *file = _fdopen(file_descriptor, "rb");
            m_ifs = std::ifstream(file);
        }
        else if (handle != nullptr)
        {
            CloseHandle(handle);
        }
    }

    ~InFileStreamImp()
    {
        if (m_ifs.is_open())
            closeImp();
    }

    bool isOpenImp() const { return m_ifs.is_open(); }

    void closeImp() { m_ifs.close(); }

    std::istream &getStreamImp() { return m_ifs; }

private:
    std::ifstream m_ifs;
};

#else

class InFileStreamImp : public std::ifstream
{
public:
    InFileStreamImp(const std::string &fileName) : std::ifstream(fileName) {}

    ~InFileStreamImp()
    {
        if (is_open())
            closeImp();
    }

    bool isOpenImp() const { return is_open(); }

    void closeImp() { close(); }

    std::istream &getStreamImp() { return *this; }
};

#endif

InFileStream::InFileStream(const std::string &fileName) : m_imp(new InFileStreamImp(fileName))
{
}

InFileStream::~InFileStream()
{
    delete m_imp;
}

bool InFileStream::isOpen() const
{
    return m_imp->isOpenImp();
}

void InFileStream::close()
{
    m_imp->closeImp();
}

std::istream &InFileStream::getStream()
{
    return m_imp->getStreamImp();
}
