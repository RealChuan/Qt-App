// Copyright (C) 2019 The Qt Company Ltd.
// Copyright (C) 2019 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "camelcasecursor.h"
#include "multitextcursor.h"

#include <QLineEdit>
#include <QPlainTextEdit>

namespace GUI {

template<typename C, typename E>
auto moveCursor(C *cursor, E *edit, QTextCursor::MoveOperation direction, QTextCursor::MoveMode mode) -> bool;

template<>
auto moveCursor(QTextCursor *cursor, QPlainTextEdit *, QTextCursor::MoveOperation direction,
                QTextCursor::MoveMode mode) -> bool
{
    return cursor->movePosition(direction, mode);
}

template<typename C>
auto moveCursor(C *, QLineEdit *edit, QTextCursor::MoveOperation direction, QTextCursor::MoveMode mode) -> bool
{
    bool mark = (mode == QTextCursor::KeepAnchor);
    switch (direction) {
    case QTextCursor::Left:
        edit->cursorBackward(mark);
        break;
    case QTextCursor::WordLeft:
        edit->cursorWordBackward(mark);
        break;
    case QTextCursor::Right:
        edit->cursorForward(mark);
        break;
    case QTextCursor::WordRight:
        edit->cursorWordForward(mark);
        break;
    default:
        return false;
    }
    return edit->cursorPosition() > 0 && edit->cursorPosition() < edit->text().size();
}

template<typename C, typename E>
auto charUnderCursor(C *cursor, E *edit) -> QChar;

template<>
auto charUnderCursor(QTextCursor *cursor, QPlainTextEdit *edit) -> QChar
{
    return edit->document()->characterAt(cursor->position());
}

template<typename C>
auto charUnderCursor(C *, QLineEdit *edit) -> QChar
{
    const int pos = edit->cursorPosition();
    if (pos < 0 || pos >= edit->text().length())
        return QChar::Null;

    return edit->text().at(pos);
};

template<typename C, typename E>
auto position(C *cursor, E *edit) -> int;

template<>
auto position(QTextCursor *cursor, QPlainTextEdit *) -> int
{
    return cursor->position();
}

template<typename C>
auto position(C *, QLineEdit *edit) -> int
{
    return edit->cursorPosition();
}

enum class Input {
    Upper,
    Lower,
    Underscore,
    Space,
    Other
};

template<typename C, typename E>
auto camelCaseLeft(C *cursor, E *edit, QTextCursor::MoveMode mode) -> bool
{
    int state = 0;

    if (!moveCursor(cursor, edit, QTextCursor::Left, mode))
        return false;

    for (;;) {
        QChar c = charUnderCursor(cursor, edit);
        Input input = Input::Other;
        if (c.isUpper())
            input = Input::Upper;
        else if (c.isLower() || c.isDigit())
            input = Input::Lower;
        else if (c == '_')
            input = Input::Underscore;
        else if (c.isSpace() && c != QChar::ParagraphSeparator)
            input = Input::Space;
        else
            input = Input::Other;

        switch (state) {
        case 0:
            switch (input) {
            case Input::Upper:
                state = 1;
                break;
            case Input::Lower:
                state = 2;
                break;
            case Input::Underscore:
                state = 3;
                break;
            case Input::Space:
                state = 4;
                break;
            default:
                moveCursor(cursor, edit, QTextCursor::Right, mode);
                return moveCursor(cursor, edit, QTextCursor::WordLeft, mode);
            }
            break;
        case 1:
            switch (input) {
            case Input::Upper:
                break;
            default:
                return moveCursor(cursor, edit, QTextCursor::Right, mode);
                return true;
            }
            break;
        case 2:
            switch (input) {
            case Input::Upper:
                return true;
            case Input::Lower:
                break;
            default:
                return moveCursor(cursor, edit, QTextCursor::Right, mode);
                return true;
            }
            break;
        case 3:
            switch (input) {
            case Input::Underscore:
                break;
            case Input::Upper:
                state = 1;
                break;
            case Input::Lower:
                state = 2;
                break;
            default:
                moveCursor(cursor, edit, QTextCursor::Right, mode);
                return true;
            }
            break;
        case 4:
            switch (input) {
            case Input::Space:
                break;
            case Input::Upper:
                state = 1;
                break;
            case Input::Lower:
                state = 2;
                break;
            case Input::Underscore:
                state = 3;
                break;
            default:
                return moveCursor(cursor, edit, QTextCursor::Right, mode);
                if (position(cursor, edit) == 0)
                    return true;
                return moveCursor(cursor, edit, QTextCursor::WordLeft, mode);
            }
        }

        if (!moveCursor(cursor, edit, QTextCursor::Left, mode))
            return true;
    }
}

template<typename C, typename E>
auto camelCaseRight(C *cursor, E *edit, QTextCursor::MoveMode mark) -> bool
{
    int state = 0;

    for (;;) {
        QChar c = charUnderCursor(cursor, edit);
        Input input = Input::Other;
        if (c.isUpper())
            input = Input::Upper;
        else if (c.isLower() || c.isDigit())
            input = Input::Lower;
        else if (c == '_')
            input = Input::Underscore;
        else if (c.isSpace() && c != QChar::ParagraphSeparator)
            input = Input::Space;
        else
            input = Input::Other;

        switch (state) {
        case 0:
            switch (input) {
            case Input::Upper:
                state = 4;
                break;
            case Input::Lower:
                state = 1;
                break;
            case Input::Underscore:
                state = 6;
                break;
            default:
                return moveCursor(cursor, edit, QTextCursor::WordRight, mark);
            }
            break;
        case 1:
            switch (input) {
            case Input::Upper:
                return true;
            case Input::Lower:
                break;
            case Input::Underscore:
                state = 6;
                break;
            case Input::Space:
                state = 7;
                break;
            default:
                return true;
            }
            break;
        case 2:
            switch (input) {
            case Input::Upper:
                break;
            case Input::Lower:
                moveCursor(cursor, edit, QTextCursor::Left, mark);
                return true;
            case Input::Underscore:
                state = 6;
                break;
            case Input::Space:
                state = 7;
                break;
            default:
                return true;
            }
            break;
        case 4:
            switch (input) {
            case Input::Upper:
                state = 2;
                break;
            case Input::Lower:
                state = 1;
                break;
            case Input::Underscore:
                state = 6;
                break;
            case Input::Space:
                state = 7;
                break;
            default:
                return true;
            }
            break;
        case 6:
            switch (input) {
            case Input::Underscore:
                break;
            case Input::Space:
                state = 7;
                break;
            default:
                return true;
            }
            break;
        case 7:
            switch (input) {
            case Input::Space:
                break;
            default:
                return true;
            }
            break;
        }
        if (!moveCursor(cursor, edit, QTextCursor::Right, mark))
            return false;
    }
}

auto CamelCaseCursor::left(QTextCursor *cursor, QPlainTextEdit *edit, QTextCursor::MoveMode mode) -> bool
{
    return camelCaseLeft(cursor, edit, mode);
}

auto CamelCaseCursor::left(MultiTextCursor *cursor, QPlainTextEdit *edit, QTextCursor::MoveMode mode) -> bool
{
    bool result = false;
    for (QTextCursor &c : *cursor)
        result |= CamelCaseCursor::left(&c, edit, mode);
    cursor->mergeCursors();
    return result;
}

auto CamelCaseCursor::left(QLineEdit *edit, QTextCursor::MoveMode mode) -> bool
{
    QTextCursor temp;
    return camelCaseLeft(&temp, edit, mode);
}

auto CamelCaseCursor::right(QTextCursor *cursor, QPlainTextEdit *edit, QTextCursor::MoveMode mode) -> bool
{
    return camelCaseRight(cursor, edit, mode);
}

auto CamelCaseCursor::right(MultiTextCursor *cursor, QPlainTextEdit *edit, QTextCursor::MoveMode mode) -> bool
{
    bool result = false;
    for (QTextCursor &c : *cursor)
        result |= CamelCaseCursor::right(&c, edit, mode);
    cursor->mergeCursors();
    return result;
}

auto CamelCaseCursor::right(QLineEdit *edit, QTextCursor::MoveMode mode) -> bool
{
    QTextCursor temp;
    return camelCaseRight(&temp, edit, mode);
}

} // namespace GUI
