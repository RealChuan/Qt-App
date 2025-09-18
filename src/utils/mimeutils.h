// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "utils_global.h"

#include <utils/mimetypes2/mimemagicrule_p.h>
#include <utils/mimetypes2/mimetype.h>

#include <functional>

namespace Utils {

class FilePath;

// Wrapped QMimeDataBase functions
UTILS_EXPORT MimeType mimeTypeForName(const QString &nameOrAlias);

enum class MimeMatchMode {
    MatchDefault = 0x0,
    MatchExtension = 0x1,
    MatchContent = 0x2,
    MatchDefaultAndRemote = 0x3
};

UTILS_EXPORT MimeType mimeTypeForFile(const QString &fileName,
                                      MimeMatchMode mode = MimeMatchMode::MatchDefault);
UTILS_EXPORT MimeType mimeTypeForFile(const FilePath &filePath,
                                      MimeMatchMode mode = MimeMatchMode::MatchDefault);
UTILS_EXPORT QList<MimeType> mimeTypesForFileName(const QString &fileName);
UTILS_EXPORT MimeType mimeTypeForData(const QByteArray &data);
UTILS_EXPORT QList<MimeType> allMimeTypes();

// Qt Creator additions
// For debugging purposes.
enum class MimeStartupPhase {
    BeforeInitialize,
    PluginsLoading,
    PluginsInitializing,        // Register up to here.
    PluginsDelayedInitializing, // Use from here on.
    UpAndRunning
};

UTILS_EXPORT void setMimeStartupPhase(MimeStartupPhase);
UTILS_EXPORT void addMimeInitializer(const std::function<void()> &init);
UTILS_EXPORT void addMimeTypes(const QString &id, const QByteArray &data);
UTILS_EXPORT QMap<int, QList<MimeMagicRule>> magicRulesForMimeType(
    const MimeType &mimeType); // priority -> rules
UTILS_EXPORT void setGlobPatternsForMimeType(const MimeType &mimeType, const QStringList &patterns);
UTILS_EXPORT void setMagicRulesForMimeType(
    const MimeType &mimeType, const QMap<int, QList<MimeMagicRule>> &rules); // priority -> rules

// visits all parents breadth-first
// visitor should return false to break the loop, true to continue
UTILS_EXPORT void visitMimeParents(const MimeType &mimeType,
                                   const std::function<bool(const MimeType &mimeType)> &visitor);
} // namespace Utils
