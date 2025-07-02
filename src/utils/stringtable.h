// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

namespace Utils::StringTable {

UTILS_EXPORT QString insert(const QString &string);
UTILS_EXPORT void scheduleGC();

} // namespace Utils::StringTable
