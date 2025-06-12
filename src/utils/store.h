// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "result.h"
#include "storekey.h"

#include <QMap>
#include <QVariant>

namespace Utils {

class QtcSettings;

using KeyList = QList<Key>;

using Store = QMap<Key, QVariant>;
using OldStore = QMap<QByteArray, QVariant>;

UTILS_EXPORT KeyList keysFromStrings(const QStringList &list);
UTILS_EXPORT QStringList stringsFromKeys(const KeyList &list);

UTILS_EXPORT QVariant variantFromStore(const Store &store);
UTILS_EXPORT Store storeFromVariant(const QVariant &value);

UTILS_EXPORT Store storeFromMap(const QVariantMap &map);
UTILS_EXPORT QVariantMap mapFromStore(const Store &store);

UTILS_EXPORT bool isStore(const QVariant &value);

UTILS_EXPORT Key numberedKey(const Key &key, int number);

UTILS_EXPORT Result<Store> storeFromJson(const QByteArray &json);
UTILS_EXPORT QByteArray jsonFromStore(const Store &store);

// These recursively change type.
UTILS_EXPORT QVariant storeEntryFromMapEntry(const QVariant &value);
UTILS_EXPORT QVariant mapEntryFromStoreEntry(const QVariant &value);

// Don't use in new code.
UTILS_EXPORT Store storeFromSettings(const Key &groupKey, QtcSettings *s);
UTILS_EXPORT void storeToSettings(const Key &groupKey, QtcSettings *s, const Store &store);
UTILS_EXPORT void storeToSettingsWithDefault(const Key &groupKey,
                                             QtcSettings *s,
                                             const Store &store,
                                             const Store &defaultStore);

} // namespace Utils

Q_DECLARE_METATYPE(Utils::Store)
Q_DECLARE_METATYPE(Utils::OldStore)
