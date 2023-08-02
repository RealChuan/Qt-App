// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "mimetypeparser_p.h"

#include "mimemagicrulematcher_p.h"
#include "mimetype_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QStack>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

namespace Utils {

// XML tags in MIME files
static const char mimeInfoTagC[] = "mime-info";
static const char mimeTypeTagC[] = "mime-type";
static const char mimeTypeAttributeC[] = "type";
static const char subClassTagC[] = "sub-class-of";
static const char commentTagC[] = "comment";
static const char genericIconTagC[] = "generic-icon";
static const char iconTagC[] = "icon";
static const char nameAttributeC[] = "name";
static const char globTagC[] = "glob";
static const char globDeleteAllTagC[] = "glob-deleteall";
static const char aliasTagC[] = "alias";
static const char patternAttributeC[] = "pattern";
static const char weightAttributeC[] = "weight";
static const char caseSensitiveAttributeC[] = "case-sensitive";
static const char localeAttributeC[] = "xml:lang";

static const char magicTagC[] = "magic";
static const char priorityAttributeC[] = "priority";

static const char matchTagC[] = "match";
static const char matchValueAttributeC[] = "value";
static const char matchTypeAttributeC[] = "type";
static const char matchOffsetAttributeC[] = "offset";
static const char matchMaskAttributeC[] = "mask";

/*!
    \class MimeTypeParser
    \inmodule QtCore
    \internal
    \brief The MimeTypeParser class parses MIME types, and builds a MIME database hierarchy by adding to MimeDatabase.

    Populates MimeDataBase

    \sa MimeDatabase, MimeMagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa MimeTypeParser
*/

/*!
    \class MimeTypeParserBase
    \inmodule QtCore
    \internal
    \brief The MimeTypeParserBase class parses for a sequence of <mime-type> in a generic way.

    Calls abstract handler function process for MimeType it finds.

    \sa MimeDatabase, MimeMagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa MimeTypeParser
*/

/*!
    \fn virtual bool MimeTypeParserBase::process(const MimeType &t, QString *errorMessage) = 0;
    Overwrite to process the sequence of parsed data
*/

MimeTypeParserBase::ParseState MimeTypeParserBase::nextState(ParseState currentState,
                                                             QStringView startElement)
{
    switch (currentState) {
    case ParseBeginning:
        if (startElement == QLatin1String(mimeInfoTagC))
            return ParseMimeInfo;
        if (startElement == QLatin1String(mimeTypeTagC))
            return ParseMimeType;
        return ParseError;
    case ParseMimeInfo:
        return startElement == QLatin1String(mimeTypeTagC) ? ParseMimeType : ParseError;
    case ParseMimeType:
    case ParseComment:
    case ParseGenericIcon:
    case ParseIcon:
    case ParseGlobPattern:
    case ParseGlobDeleteAll:
    case ParseSubClass:
    case ParseAlias:
    case ParseOtherMimeTypeSubTag:
    case ParseMagicMatchRule:
        if (startElement == QLatin1String(mimeTypeTagC)) // Sequence of <mime-type>
            return ParseMimeType;
        if (startElement == QLatin1String(commentTagC))
            return ParseComment;
        if (startElement == QLatin1String(genericIconTagC))
            return ParseGenericIcon;
        if (startElement == QLatin1String(iconTagC))
            return ParseIcon;
        if (startElement == QLatin1String(globTagC))
            return ParseGlobPattern;
        if (startElement == QLatin1String(globDeleteAllTagC))
            return ParseGlobDeleteAll;
        if (startElement == QLatin1String(subClassTagC))
            return ParseSubClass;
        if (startElement == QLatin1String(aliasTagC))
            return ParseAlias;
        if (startElement == QLatin1String(magicTagC))
            return ParseMagic;
        if (startElement == QLatin1String(matchTagC))
            return ParseMagicMatchRule;
        return ParseOtherMimeTypeSubTag;
    case ParseMagic:
        if (startElement == QLatin1String(matchTagC))
            return ParseMagicMatchRule;
        break;
    case ParseError: break;
    }
    return ParseError;
}

// Parse int number from an (attribute) string
bool MimeTypeParserBase::parseNumber(QStringView n, int *target, QString *errorMessage)
{
    bool ok;
    *target = n.toInt(&ok);
    if (Q_UNLIKELY(!ok)) {
        if (errorMessage) {
            *errorMessage = QLatin1String("Not a number '");
            *errorMessage += n;
            *errorMessage += QLatin1String("'.");
        }
        return false;
    }
    return true;
}

#ifndef QT_NO_XMLSTREAMREADER
struct CreateMagicMatchRuleResult
{
    QString errorMessage; // must be first
    MimeMagicRule rule;

    CreateMagicMatchRuleResult(QStringView type,
                               QStringView value,
                               QStringView offsets,
                               QStringView mask)
        : errorMessage()
        , rule(type.toString(), value.toUtf8(), offsets.toString(), mask.toLatin1(), &errorMessage)
    {}
};

static CreateMagicMatchRuleResult createMagicMatchRule(const QXmlStreamAttributes &atts)
{
    const auto type = atts.value(QLatin1String(matchTypeAttributeC));
    const auto value = atts.value(QLatin1String(matchValueAttributeC));
    const auto offsets = atts.value(QLatin1String(matchOffsetAttributeC));
    const auto mask = atts.value(QLatin1String(matchMaskAttributeC));
    return CreateMagicMatchRuleResult(type, value, offsets, mask);
}
#endif

bool MimeTypeParserBase::parse(QIODevice *dev, const QString &fileName, QString *errorMessage)
{
#ifdef QT_NO_XMLSTREAMREADER
    Q_UNUSED(dev);
    if (errorMessage)
        *errorMessage = QString::fromLatin1("QXmlStreamReader is not available, cannot parse '%1'.")
                            .arg(fileName);
    return false;
#else
    MimeTypePrivate data;
    data.loaded = true;
    int priority = 50;
    QStack<MimeMagicRule *> currentRules; // stack for the nesting of rules
    QList<MimeMagicRule> rules;           // toplevel rules
    QXmlStreamReader reader(dev);
    ParseState ps = ParseBeginning;
    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::StartElement: {
            ps = nextState(ps, reader.name());
            const QXmlStreamAttributes atts = reader.attributes();
            switch (ps) {
            case ParseMimeType: { // start parsing a MIME type name
                const QString name = atts.value(QLatin1String(mimeTypeAttributeC)).toString();
                if (name.isEmpty()) {
                    reader.raiseError(QStringLiteral("Missing 'type'-attribute"));
                } else {
                    data.name = name;
                }
            } break;
            case ParseGenericIcon:
                data.genericIconName = atts.value(QLatin1String(nameAttributeC)).toString();
                break;
            case ParseIcon:
                data.iconName = atts.value(QLatin1String(nameAttributeC)).toString();
                break;
            case ParseGlobPattern: {
                const QString pattern = atts.value(QLatin1String(patternAttributeC)).toString();
                unsigned weight = atts.value(QLatin1String(weightAttributeC)).toInt();
                const bool caseSensitive = atts.value(QLatin1String(caseSensitiveAttributeC))
                                           == QLatin1String("true");

                if (weight == 0)
                    weight = MimeGlobPattern::DefaultWeight;

                Q_ASSERT(!data.name.isEmpty());
                const MimeGlobPattern glob(pattern,
                                           data.name,
                                           weight,
                                           caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
                if (!process(glob, errorMessage)) // for actual glob matching
                    return false;
                data.addGlobPattern(pattern);     // just for MimeType::globPatterns()
            } break;
            case ParseGlobDeleteAll: data.globPatterns.clear(); break;
            case ParseSubClass: {
                const QString inheritsFrom = atts.value(QLatin1String(mimeTypeAttributeC)).toString();
                if (!inheritsFrom.isEmpty())
                    processParent(data.name, inheritsFrom);
            } break;
            case ParseComment: {
                // comments have locale attributes.
                QString locale = atts.value(QLatin1String(localeAttributeC)).toString();
                const QString comment = reader.readElementText();
                if (locale.isEmpty())
                    locale = QString::fromLatin1("default");
                data.localeComments.insert(locale, comment);
            } break;
            case ParseAlias: {
                const QString alias = atts.value(QLatin1String(mimeTypeAttributeC)).toString();
                if (!alias.isEmpty())
                    processAlias(alias, data.name);
            } break;
            case ParseMagic: {
                priority = 50;
                const auto priorityS = atts.value(QLatin1String(priorityAttributeC));
                if (!priorityS.isEmpty()) {
                    if (!parseNumber(priorityS, &priority, errorMessage))
                        return false;
                }
                currentRules.clear();
                //qDebug() << "MAGIC start for mimetype" << data.name;
            } break;
            case ParseMagicMatchRule: {
                auto result = createMagicMatchRule(atts);
                if (Q_UNLIKELY(!result.rule.isValid()))
                    qWarning("MimeDatabase: Error parsing %ls\n%ls",
                             qUtf16Printable(fileName),
                             qUtf16Printable(result.errorMessage));
                QList<MimeMagicRule> *ruleList;
                if (currentRules.isEmpty())
                    ruleList = &rules;
                else // nest this rule into the proper parent
                    ruleList = &currentRules.top()->m_subMatches;
                ruleList->append(std::move(result.rule));
                //qDebug() << " MATCH added. Stack size was" << currentRules.size();
                currentRules.push(&ruleList->last());
                break;
            }
            case ParseError: {
                QString error = QLatin1String("Unexpected element <");
                error += reader.name();
                error += QLatin1Char('>');
                reader.raiseError(error);
                break;
            }
            default: break;
            }
        } break;
        // continue switch QXmlStreamReader::Token...
        case QXmlStreamReader::EndElement: // Finished element
        {
            const auto elementName = reader.name();
            if (elementName == QLatin1String(mimeTypeTagC)) {
                if (!process(MimeType(data), errorMessage))
                    return false;
                data.clear();
            } else if (elementName == QLatin1String(matchTagC)) {
                // Closing a <match> tag, pop stack
                currentRules.pop();
                //qDebug() << " MATCH closed. Stack size is now" << currentRules.size();
            } else if (elementName == QLatin1String(magicTagC)) {
                //qDebug() << "MAGIC ended, we got" << rules.count() << "rules, with prio" << priority;
                // Finished a <magic> sequence
                MimeMagicRuleMatcher ruleMatcher(data.name, priority);
                ruleMatcher.addRules(rules);
                processMagicMatcher(ruleMatcher);
                rules.clear();
            }
            break;
        }
        default: break;
        }
    }

    if (Q_UNLIKELY(reader.hasError())) {
        if (errorMessage) {
            *errorMessage
                = QString::asprintf("An error has been encountered at line %lld of %ls: %ls:",
                                    reader.lineNumber(),
                                    qUtf16Printable(fileName),
                                    qUtf16Printable(reader.errorString()));
        }
        return false;
    }

    return true;
#endif //QT_NO_XMLSTREAMREADER
}

} // namespace Utils
