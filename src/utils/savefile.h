// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include "filepath.h"

#include <QFile>

#include <memory>

namespace Utils {

class UTILS_EXPORT SaveFile : public QFile
{
public:
    explicit SaveFile(const FilePath &filePath);
    ~SaveFile() override;

    bool open(OpenMode flags = QIODevice::WriteOnly) override;

    void rollback();
    bool commit();

    static void initializeUmask();

private:
    const FilePath m_finalFilePath;
    std::unique_ptr<TemporaryFilePath> m_tempFile;
    bool m_finalized = true;
};

} // namespace Utils
