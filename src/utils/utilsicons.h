// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include "icon.h"

namespace Utils {
namespace Icons {

UTILS_EXPORT std::optional<Icon> fromString(const QString &name);

UTILS_EXPORT extern const Icon EDIT_CLEAR;
UTILS_EXPORT extern const Icon OK;
UTILS_EXPORT extern const Icon NOTLOADED;
UTILS_EXPORT extern const Icon BROKEN;

UTILS_EXPORT extern const Icon INFO;
UTILS_EXPORT extern const Icon WARNING;
UTILS_EXPORT extern const Icon CRITICAL;

} // namespace Icons

namespace CodeModelIcon {

enum Type {
    Class = 0,
    Struct,
    Enum,
    Enumerator,
    FuncPublic,
    FuncProtected,
    FuncPrivate,
    FuncPublicStatic,
    FuncProtectedStatic,
    FuncPrivateStatic,
    Namespace,
    VarPublic,
    VarProtected,
    VarPrivate,
    VarPublicStatic,
    VarProtectedStatic,
    VarPrivateStatic,
    Signal,
    SlotPublic,
    SlotProtected,
    SlotPrivate,
    Keyword,
    Macro,
    Property,
    Unknown
};

} // namespace CodeModelIcon
} // namespace Utils
