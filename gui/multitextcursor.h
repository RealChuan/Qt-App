// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "gui_global.hpp"

#include <QKeySequence>
#include <QTextCursor>

QT_BEGIN_NAMESPACE
class QKeyEvent;
class QPlainTextEdit;
QT_END_NAMESPACE

namespace GUI {

class GUI_EXPORT MultiTextCursor
{
public:
    MultiTextCursor();
    explicit MultiTextCursor(const QList<QTextCursor> &cursors);

    /// replace all cursors with \param cursors and the last one will be the new main cursors
    void setCursors(const QList<QTextCursor> &cursors);
    [[nodiscard]] auto cursors() const -> const QList<QTextCursor>;

    /// \returns whether this multi cursor contains any cursor
    [[nodiscard]] auto isNull() const -> bool;
    /// \returns whether this multi cursor contains more than one cursor
    [[nodiscard]] auto hasMultipleCursors() const -> bool;
    /// \returns the number of cursors handled by this cursor
    [[nodiscard]] auto cursorCount() const -> int;

    /// the \param cursor that is appended by added by \brief addCursor
    /// will be interpreted as the new main cursor
    void addCursor(const QTextCursor &cursor);
    void addCursors(const QList<QTextCursor> &cursors);
    /// convenience function that removes the old main cursor and appends
    /// \param cursor as the new main cursor
    void replaceMainCursor(const QTextCursor &cursor);
    /// \returns the main cursor
    [[nodiscard]] auto mainCursor() const -> QTextCursor;
    /// \returns the main cursor and removes it from this multi cursor
    auto takeMainCursor() -> QTextCursor;

    void beginEditBlock();
    void endEditBlock();
    /// merges overlapping cursors together
    void mergeCursors();

    /// applies the move key event \param e to all cursors in this multi cursor
    auto handleMoveKeyEvent(QKeyEvent *e, QPlainTextEdit *edit, bool camelCaseNavigationEnabled) -> bool;
    /// applies the move \param operation to all cursors in this multi cursor \param n times
    /// with the move \param mode
    void movePosition(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode, int n = 1);

    /// \returns whether any cursor has a selection
    [[nodiscard]] auto hasSelection() const -> bool;
    /// \returns the selected text of all cursors that have a selection separated by
    /// a newline character
    [[nodiscard]] auto selectedText() const -> QString;
    /// removes the selected text of all cursors that have a selection from the document
    void removeSelectedText();

    /// inserts \param text into all cursors, potentially removing correctly selected text
    void insertText(const QString &text, bool selectNewText = false);

    auto operator==(const MultiTextCursor &other) const -> bool;
    auto operator!=(const MultiTextCursor &other) const -> bool;

    using iterator = QList<QTextCursor>::iterator;
    using const_iterator = QList<QTextCursor>::const_iterator;

    iterator begin() { return m_cursors.begin(); }
    iterator end() { return m_cursors.end(); }
    [[nodiscard]] const_iterator begin() const { return m_cursors.begin(); }
    [[nodiscard]] const_iterator end() const { return m_cursors.end(); }
    [[nodiscard]] const_iterator constBegin() const { return m_cursors.constBegin(); }
    [[nodiscard]] const_iterator constEnd() const { return m_cursors.constEnd(); }

    static auto multiCursorAddEvent(QKeyEvent *e, QKeySequence::StandardKey matchKey) -> bool;

private:
    QList<QTextCursor> m_cursors;
};

} // namespace GUI
