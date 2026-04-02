// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "TestUtils.h"
#include <QDirIterator>

namespace tutl
{

InFileStream::Ptr openTestFile(const std::string &fileName)
{
    EXPECT_TRUE(QFile::exists(utl::toQStr(fileName)));
    auto ifs = InFileStream::make(fileName);
    EXPECT_TRUE(ifs);
    EXPECT_TRUE(ifs->isOpen());
    EXPECT_TRUE(ifs->getStream().good());
    return ifs;
}

void copyResourcesToCurrentDir()
{
    QDirIterator it(":/resources", QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString resPath = it.next();
        QString destPath = resPath;
        destPath.replace(":/resources/", "");

        QFileInfo info(resPath);
        if (info.isFile())
        {
            if (QFile::exists(destPath))
            {
                QFile::remove(destPath);
            }
            QFile::copy(resPath, destPath);
        }
        else if (info.isDir())
        {
            QDir().mkpath(destPath);
        }
    }
}

} // namespace tutl