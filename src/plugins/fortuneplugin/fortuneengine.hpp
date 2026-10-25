#pragma once

#include "fortunecontent.hpp"

#include <utils/result.hpp>

#include <QDate>
#include <QDateTime>
#include <QString>
#include <QStringList>

namespace Fortune {

struct Profile
{
    QString name;
    QDate birthDate;
    // 0 = unknown, 1..12 = Zi .. Hai.
    int birthHour = 0;

    bool operator==(const Profile &) const = default;
};

struct BirthChart
{
    int hexagram = 1;
    int movingLine = 1;
    Element element = Element::Earth;

    bool operator==(const BirthChart &) const = default;
};

struct HistoryEntry
{
    QDateTime castAt;
    int hexagram = 1;
    int movingLine = 1;
    Grade grade = Grade::Fair;

    bool operator==(const HistoryEntry &) const = default;
};

struct Reading
{
    QDateTime castAt;
    int hexagram = 1;
    int movingLine = 1;
    int changedHexagram = 1;
    Grade grade = Grade::Fair;
    QString judgment;
    QString headline;
    QString reading;
    QStringList doList;
    QStringList avoidList;
    int luckyNumber = 1;
    QString luckyColor;
    QString luckyElement;
    int birthHexagram = 1;

    bool operator==(const Reading &) const = default;
};

// The engine never reads the clock: the caller passes the moment of the cast.
[[nodiscard]] auto cast(const Profile &profile, const QDateTime &moment) -> Utils::Result<Reading>;
[[nodiscard]] auto birthChart(const Profile &profile) -> Utils::Result<BirthChart>;

} // namespace Fortune
