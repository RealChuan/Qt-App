// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "gui_global.hpp"

#include <QSortFilterProxyModel>

namespace GUI {

class GUI_EXPORT CategorySortFilterModel : public QSortFilterProxyModel
{
public:
    explicit CategorySortFilterModel(QObject *parent = nullptr);

    // "New" items will always be accepted, regardless of the filter.
    void setNewItemRole(int role);

protected:
    [[nodiscard]] auto filterAcceptsRow(int source_row, const QModelIndex &source_parent) const -> bool override;

private:
    int m_newItemRole = -1;
};

} // namespace GUI
