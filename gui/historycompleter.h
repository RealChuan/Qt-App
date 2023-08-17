// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "gui_global.hpp"

#include <QCompleter>

namespace Utils {
class QtcSettings;
}

namespace GUI {

class HistoryCompleterPrivate;
class GUI_EXPORT HistoryCompleter : public QCompleter
{
    Q_OBJECT

public:
    static void setSettings(Utils::QtcSettings *settings);
    explicit HistoryCompleter(const QString &historyKey, QObject *parent = nullptr);
    auto removeHistoryItem(int index) -> bool;
    [[nodiscard]] auto historyItem() const -> QString;
    [[nodiscard]] auto hasHistory() const -> bool { return historySize() > 0; }
    static auto historyExistsFor(const QString &historyKey) -> bool;

private:
    ~HistoryCompleter() override;
    [[nodiscard]] auto historySize() const -> int;
    [[nodiscard]] auto maximalHistorySize() const -> int;
    void setMaximalHistorySize(int numberOfEntries);

public Q_SLOTS:
    void clearHistory();
    void addEntry(const QString &str);

private:
    HistoryCompleterPrivate *d;
};

} // namespace GUI
