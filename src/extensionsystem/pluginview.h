// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "extensionsystem_global.h"

#include <gui/treemodel.h>

#include <QWidget>

#include <unordered_map>

namespace GUI {
class CategorySortFilterModel;
class TreeView;
} // namespace GUI

namespace ExtensionSystem {

class PluginSpec;

namespace Internal {
class CollectionItem;
class PluginItem;
} // Internal

class EXTENSIONSYSTEM_EXPORT PluginView : public QWidget
{
    Q_OBJECT

public:
    explicit PluginView(QWidget *parent = nullptr);
    ~PluginView() override;

    [[nodiscard]] auto currentPlugin() const -> PluginSpec *;
    void setFilter(const QString &filter);
    void cancelChanges();

signals:
    void currentPluginChanged(ExtensionSystem::PluginSpec *spec);
    void pluginActivated(ExtensionSystem::PluginSpec *spec);
    void pluginSettingsChanged(ExtensionSystem::PluginSpec *spec);

private:
    [[nodiscard]] auto pluginForIndex(const QModelIndex &index) const -> PluginSpec *;
    void updatePlugins();
    auto setPluginsEnabled(const QSet<PluginSpec *> &plugins, bool enable) -> bool;

    GUI::TreeView *m_categoryView;
    GUI::TreeModel<GUI::TreeItem, Internal::CollectionItem, Internal::PluginItem> *m_model;
    GUI::CategorySortFilterModel *m_sortModel;
    std::unordered_map<PluginSpec *, bool> m_affectedPlugins;

    friend class Internal::CollectionItem;
    friend class Internal::PluginItem;
};

} // namespae ExtensionSystem
