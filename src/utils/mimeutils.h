// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "utils_global.h"

#include <utils/mimetypes/mimemagicrule_p.h>
#include <utils/mimetypes/mimetype.h>

#include <functional>

namespace Utils {

// Wrapped QMimeDataBase functions
UTILS_EXPORT auto mimeTypeForName(const QString &nameOrAlias) -> MimeType;

enum class MimeMatchMode {
    MatchDefault = 0x0,
    MatchExtension = 0x1,
    MatchContent = 0x2,
    MatchDefaultAndRemote = 0x3
};

UTILS_EXPORT auto mimeTypeForFile(const QString &fileName,
                                  MimeMatchMode mode = MimeMatchMode::MatchDefault) -> MimeType;
UTILS_EXPORT auto mimeTypesForFileName(const QString &fileName) -> QList<MimeType>;
UTILS_EXPORT auto mimeTypeForData(const QByteArray &data) -> MimeType;
UTILS_EXPORT auto allMimeTypes() -> QList<MimeType>;

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
UTILS_EXPORT void addMimeTypes(const QString &id, const QByteArray &data);
UTILS_EXPORT auto magicRulesForMimeType(const MimeType &mimeType)
    -> QMap<int, QList<MimeMagicRule>>; // priority -> rules
UTILS_EXPORT void setGlobPatternsForMimeType(const MimeType &mimeType, const QStringList &patterns);
UTILS_EXPORT void setMagicRulesForMimeType(
    const MimeType &mimeType, const QMap<int, QList<MimeMagicRule>> &rules); // priority -> rules

// visits all parents breadth-first
// visitor should return false to break the loop, true to continue
UTILS_EXPORT void visitMimeParents(const MimeType &mimeType,
                                   const std::function<bool(const MimeType &mimeType)> &visitor);
} // namespace Utils
