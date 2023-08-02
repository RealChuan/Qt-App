// Copyright (C) 2019 The Qt Company Ltd.
// Copyright (C) 2019 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "gui_global.hpp"

#include <QTextCursor>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPlainTextEdit;
QT_END_NAMESPACE

namespace GUI {

class MultiTextCursor;

class GUI_EXPORT CamelCaseCursor
{
public:
    static bool left(QTextCursor *cursor, QPlainTextEdit *edit, QTextCursor::MoveMode mode);
    static bool left(MultiTextCursor *cursor, QPlainTextEdit *edit, QTextCursor::MoveMode mode);
    static bool left(QLineEdit *edit, QTextCursor::MoveMode mode);
    static bool right(QTextCursor *cursor, QPlainTextEdit *edit, QTextCursor::MoveMode mode);
    static bool right(MultiTextCursor *cursor, QPlainTextEdit *edit, QTextCursor::MoveMode mode);
    static bool right(QLineEdit *edit, QTextCursor::MoveMode mode);
};

} // namespace GUI
