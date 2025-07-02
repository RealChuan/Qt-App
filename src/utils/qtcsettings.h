// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include "store.h"

#include <QSettings>

#include <functional>

namespace Utils {

class UTILS_EXPORT QtcSettings : private QSettings
{
public:
    using QSettings::allKeys;
    using QSettings::beginReadArray;
    using QSettings::beginWriteArray;
    using QSettings::childGroups;
    using QSettings::clear;
    using QSettings::endArray;
    using QSettings::endGroup;
    using QSettings::fileName;
    using QSettings::group;
    using QSettings::QSettings;
    using QSettings::setArrayIndex;
    using QSettings::setParent;
    using QSettings::status;
    using QSettings::sync;

    void withGroup(const Key &prefix, const std::function<void(QtcSettings *)> &function);

    void beginGroup(const Key &prefix);

    QVariant value(const Key &key) const;
    QVariant value(const Key &key, const QVariant &def) const;
    void setValue(const Key &key, const QVariant &value);
    void remove(const Key &key);
    bool contains(const Key &key) const;

    KeyList childKeys() const;

    template<typename T>
    void setValueWithDefault(const Key &key, const T &val, const T &defaultValue)
    {
        if (val == defaultValue)
            remove(key);
        else
            setValue(key, val);
    }

    template<typename T>
    void setValueWithDefault(const Key &key, const T &val)
    {
        if (val == T())
            remove(key);
        else
            setValue(key, val);
    }
};

} // namespace Utils
