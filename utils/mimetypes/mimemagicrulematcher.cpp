// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "mimemagicrulematcher_p.h"

#include "mimetype_p.h"

namespace Utils {

/*!
    \internal
    \class MimeMagicRuleMatcher
    \inmodule QtCore

    \brief The MimeMagicRuleMatcher class checks a number of rules based on operator "or".

    It is used for rules parsed from XML files.

    \sa MimeType, MimeDatabase, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa MimeTypeParserBase, MimeTypeParser
*/

MimeMagicRuleMatcher::MimeMagicRuleMatcher(const QString &mime, unsigned thePriority) :
    m_list(),
    m_priority(thePriority),
    m_mimetype(mime)
{
}

auto MimeMagicRuleMatcher::operator==(const MimeMagicRuleMatcher &other) const -> bool
{
    return m_list == other.m_list &&
           m_priority == other.m_priority;
}

void MimeMagicRuleMatcher::addRule(const MimeMagicRule &rule)
{
    m_list.append(rule);
}

void MimeMagicRuleMatcher::addRules(const QList<MimeMagicRule> &rules)
{
    m_list.append(rules);
}

auto MimeMagicRuleMatcher::magicRules() const -> QList<MimeMagicRule>
{
    return m_list;
}

// Check for a match on contents of a file
auto MimeMagicRuleMatcher::matches(const QByteArray &data) const -> bool
{
    for (const MimeMagicRule &magicRule : m_list) {
        if (magicRule.matches(data))
            return true;
    }

    return false;
}

// Return a priority value from 1..100
auto MimeMagicRuleMatcher::priority() const -> unsigned
{
    return m_priority;
}

} // namespace Utils
