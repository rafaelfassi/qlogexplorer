// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

class InFileStreamImp;

class InFileStream
{
public:
    InFileStream(const InFileStream &) = delete;
    InFileStream(InFileStream &&) = delete;
    ~InFileStream();

    bool isOpen() const;
    void close();
    std::istream &getStream();

    using Ptr = std::unique_ptr<InFileStream>;
    static InFileStream::Ptr make(const std::string &fileName) { return InFileStream::Ptr(new InFileStream(fileName)); }

private:
    InFileStream(const std::string &fileName);
    InFileStreamImp *m_imp;
};
