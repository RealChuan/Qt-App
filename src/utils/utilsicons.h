// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include "icon.h"

namespace Utils {
namespace Icons {

UTILS_EXPORT std::optional<Icon> fromString(const QString &name);

UTILS_EXPORT extern const Icon HOME;
UTILS_EXPORT extern const Icon HOME_TOOLBAR;
UTILS_EXPORT extern const Icon EDIT_CLEAR;
UTILS_EXPORT extern const Icon EDIT_CLEAR_TOOLBAR;
UTILS_EXPORT extern const Icon LOCKED_TOOLBAR;
UTILS_EXPORT extern const Icon LOCKED;
UTILS_EXPORT extern const Icon UNLOCKED_TOOLBAR;
UTILS_EXPORT extern const Icon UNLOCKED;
UTILS_EXPORT extern const Icon PINNED;
UTILS_EXPORT extern const Icon PINNED_SMALL;
UTILS_EXPORT extern const Icon NEXT;
UTILS_EXPORT extern const Icon NEXT_TOOLBAR;
UTILS_EXPORT extern const Icon PREV;
UTILS_EXPORT extern const Icon PREV_TOOLBAR;
UTILS_EXPORT extern const Icon PROJECT;
UTILS_EXPORT extern const Icon ZOOM;
UTILS_EXPORT extern const Icon ZOOM_TOOLBAR;
UTILS_EXPORT extern const Icon ZOOMIN_TOOLBAR;
UTILS_EXPORT extern const Icon ZOOMOUT_TOOLBAR;
UTILS_EXPORT extern const Icon FITTOVIEW_TOOLBAR;
UTILS_EXPORT extern const Icon OK;
UTILS_EXPORT extern const Icon NOTLOADED;
UTILS_EXPORT extern const Icon BROKEN;
UTILS_EXPORT extern const Icon BOOKMARK;
UTILS_EXPORT extern const Icon BOOKMARK_TOOLBAR;
UTILS_EXPORT extern const Icon BOOKMARK_TEXTEDITOR;
UTILS_EXPORT extern const Icon SNAPSHOT;
UTILS_EXPORT extern const Icon SNAPSHOT_TOOLBAR;
UTILS_EXPORT extern const Icon NEWSEARCH_TOOLBAR;
UTILS_EXPORT extern const Icon SETTINGS;
UTILS_EXPORT extern const Icon SETTINGS_TOOLBAR;

UTILS_EXPORT extern const Icon NEWFILE;
UTILS_EXPORT extern const Icon OPENFILE;
UTILS_EXPORT extern const Icon OPENFILE_TOOLBAR;
UTILS_EXPORT extern const Icon SAVEFILE;
UTILS_EXPORT extern const Icon SAVEFILE_TOOLBAR;

UTILS_EXPORT extern const Icon EXPORTFILE_TOOLBAR;
UTILS_EXPORT extern const Icon MULTIEXPORTFILE_TOOLBAR;

UTILS_EXPORT extern const Icon DIR;
UTILS_EXPORT extern const Icon HELP;
UTILS_EXPORT extern const Icon UNKNOWN_FILE;

UTILS_EXPORT extern const Icon UNDO;
UTILS_EXPORT extern const Icon UNDO_TOOLBAR;
UTILS_EXPORT extern const Icon REDO;
UTILS_EXPORT extern const Icon REDO_TOOLBAR;
UTILS_EXPORT extern const Icon COPY;
UTILS_EXPORT extern const Icon COPY_TOOLBAR;
UTILS_EXPORT extern const Icon PASTE;
UTILS_EXPORT extern const Icon PASTE_TOOLBAR;
UTILS_EXPORT extern const Icon CUT;
UTILS_EXPORT extern const Icon CUT_TOOLBAR;
UTILS_EXPORT extern const Icon RESET;
UTILS_EXPORT extern const Icon RESET_TOOLBAR;

UTILS_EXPORT extern const Icon ARROW_UP;
UTILS_EXPORT extern const Icon ARROW_UP_TOOLBAR;
UTILS_EXPORT extern const Icon ARROW_DOWN;
UTILS_EXPORT extern const Icon ARROW_DOWN_TOOLBAR;
UTILS_EXPORT extern const Icon MINUS_TOOLBAR;
UTILS_EXPORT extern const Icon MINUS;
UTILS_EXPORT extern const Icon PLUS_TOOLBAR;
UTILS_EXPORT extern const Icon PLUS;
UTILS_EXPORT extern const Icon MAGNIFIER;
UTILS_EXPORT extern const Icon CLEAN;
UTILS_EXPORT extern const Icon CLEAN_TOOLBAR;
UTILS_EXPORT extern const Icon RELOAD;
UTILS_EXPORT extern const Icon RELOAD_TOOLBAR;
UTILS_EXPORT extern const Icon TOGGLE_LEFT_SIDEBAR;
UTILS_EXPORT extern const Icon TOGGLE_LEFT_SIDEBAR_TOOLBAR;
UTILS_EXPORT extern const Icon TOGGLE_RIGHT_SIDEBAR;
UTILS_EXPORT extern const Icon TOGGLE_RIGHT_SIDEBAR_TOOLBAR;
UTILS_EXPORT extern const Icon CLOSE_TOOLBAR;
UTILS_EXPORT extern const Icon CLOSE_FOREGROUND;
UTILS_EXPORT extern const Icon CLOSE_BACKGROUND;
UTILS_EXPORT extern const Icon SPLIT_HORIZONTAL;
UTILS_EXPORT extern const Icon SPLIT_HORIZONTAL_TOOLBAR;
UTILS_EXPORT extern const Icon SPLIT_VERTICAL;
UTILS_EXPORT extern const Icon SPLIT_VERTICAL_TOOLBAR;
UTILS_EXPORT extern const Icon CLOSE_SPLIT_TOP;
UTILS_EXPORT extern const Icon CLOSE_SPLIT_BOTTOM;
UTILS_EXPORT extern const Icon CLOSE_SPLIT_LEFT;
UTILS_EXPORT extern const Icon CLOSE_SPLIT_RIGHT;
UTILS_EXPORT extern const Icon FILTER;
UTILS_EXPORT extern const Icon LINK;
UTILS_EXPORT extern const Icon LINK_TOOLBAR;
UTILS_EXPORT extern const Icon SORT_ALPHABETICALLY_TOOLBAR;
UTILS_EXPORT extern const Icon TOGGLE_PROGRESSDETAILS_TOOLBAR;
UTILS_EXPORT extern const Icon ONLINE;
UTILS_EXPORT extern const Icon ONLINE_TOOLBAR;
UTILS_EXPORT extern const Icon DOWNLOAD;

UTILS_EXPORT extern const Icon INFO;
UTILS_EXPORT extern const Icon INFO_TOOLBAR;
UTILS_EXPORT extern const Icon WARNING;
UTILS_EXPORT extern const Icon WARNING_TOOLBAR;
UTILS_EXPORT extern const Icon CRITICAL;
UTILS_EXPORT extern const Icon CRITICAL_TOOLBAR;

UTILS_EXPORT extern const Icon ERROR_TASKBAR;
UTILS_EXPORT extern const Icon EXPAND_ALL_TOOLBAR;
UTILS_EXPORT extern const Icon TOOLBAR_EXTENSION;
UTILS_EXPORT extern const Icon RUN_SMALL;
UTILS_EXPORT extern const Icon RUN_SMALL_TOOLBAR;
UTILS_EXPORT extern const Icon STOP_SMALL;
UTILS_EXPORT extern const Icon STOP_SMALL_TOOLBAR;
UTILS_EXPORT extern const Icon INTERRUPT_SMALL;
UTILS_EXPORT extern const Icon INTERRUPT_SMALL_TOOLBAR;
UTILS_EXPORT extern const Icon CONTINUE_SMALL;
UTILS_EXPORT extern const Icon CONTINUE_SMALL_TOOLBAR;

UTILS_EXPORT extern const Icon BOUNDING_RECT;
UTILS_EXPORT extern const Icon EYE_OPEN;
UTILS_EXPORT extern const Icon EYE_OPEN_TOOLBAR;
UTILS_EXPORT extern const Icon EYE_CLOSED_TOOLBAR;
UTILS_EXPORT extern const Icon REPLACE;
UTILS_EXPORT extern const Icon EXPAND;
UTILS_EXPORT extern const Icon EXPAND_TOOLBAR;
UTILS_EXPORT extern const Icon CLOCK_BLACK;
UTILS_EXPORT extern const Icon COLLAPSE;
UTILS_EXPORT extern const Icon COLLAPSE_TOOLBAR;
UTILS_EXPORT extern const Icon PAN_TOOLBAR;
UTILS_EXPORT extern const Icon EMPTY14;
UTILS_EXPORT extern const Icon EMPTY16;
UTILS_EXPORT extern const Icon OVERLAY_ADD;
UTILS_EXPORT extern const Icon OVERLAY_WARNING;
UTILS_EXPORT extern const Icon OVERLAY_ERROR;
UTILS_EXPORT extern const Icon RUN_FILE;
UTILS_EXPORT extern const Icon RUN_FILE_TOOLBAR;
UTILS_EXPORT extern const Icon RUN_SELECTED;
UTILS_EXPORT extern const Icon RUN_SELECTED_TOOLBAR;

UTILS_EXPORT extern const Icon CODEMODEL_ERROR;
UTILS_EXPORT extern const Icon CODEMODEL_WARNING;
UTILS_EXPORT extern const Icon CODEMODEL_DISABLED_ERROR;
UTILS_EXPORT extern const Icon CODEMODEL_DISABLED_WARNING;
UTILS_EXPORT extern const Icon CODEMODEL_FIXIT;

UTILS_EXPORT extern const Icon MACOS_TOUCHBAR_BOOKMARK;
UTILS_EXPORT extern const Icon MACOS_TOUCHBAR_CLEAR;

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

UTILS_EXPORT QIcon iconForType(Type type);

} // namespace CodeModelIcon
} // namespace Utils
