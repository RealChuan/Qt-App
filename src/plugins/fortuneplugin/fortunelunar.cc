#include "fortunelunar.hpp"

#include "fortunestrings.hpp"

#include <QString>

#include <array>
#include <cstdint>

namespace Fortune {

namespace {

constexpr int kFirstYear = 1901;
constexpr int kLastYear = 2100;
constexpr int kMonthCount = 12;

// 1901-02-19 is lunar 1901-01-01.
constexpr int kBaseYear = 1901;
constexpr int kBaseMonth = 2;
constexpr int kBaseDay = 19;

// Encoding, one word per lunar year:
//   bits 0-3   leap month number (0 = no leap month)
//   bit 4      leap month length (0 = 29 days, 1 = 30 days)
//   bits 5-16  month lengths 1..12 (0 = 29 days, 1 = 30 days)
// Generated from the .NET ChineseLunisolarCalendar and checked against known
// new year dates; see tests/fortune/tst_fortune.cpp.
constexpr std::array<std::uint32_t, 200> kYearInfo{{
    0x0ea40u, 0x1d4a0u, 0x16545u, 0x0c960u, 0x15360u, 0x154d4u, 0x0ad40u, 0x16b20u, 0x17542u,
    0x0ea40u, 0x1b4a6u, 0x164a0u, 0x14960u, 0x14975u, 0x055a0u, 0x0ad60u, 0x0b6a2u, 0x1b520u,
    0x1d257u, 0x1d240u, 0x1a4a0u, 0x1a5a5u, 0x14ac0u, 0x056c0u, 0x15ab4u, 0x0da80u, 0x1d520u,
    0x1e942u, 0x1d240u, 0x0d4c6u, 0x0a560u, 0x14ae0u, 0x12ad5u, 0x16b40u, 0x0da80u, 0x0ec33u,
    0x0e920u, 0x16277u, 0x15260u, 0x0a560u, 0x0a376u, 0x155a0u, 0x0ad40u, 0x1b4b4u, 0x17480u,
    0x16920u, 0x1a962u, 0x152a0u, 0x155a7u, 0x0a6c0u, 0x155a0u, 0x15955u, 0x0b640u, 0x1b4a0u,
    0x1d453u, 0x1a940u, 0x0b2a8u, 0x152e0u, 0x0aac0u, 0x0aea6u, 0x15aa0u, 0x0da40u, 0x0eaa4u,
    0x1d4a0u, 0x0c940u, 0x0c9e3u, 0x15360u, 0x15b47u, 0x0ad40u, 0x16d20u, 0x17645u, 0x16a40u,
    0x164a0u, 0x16564u, 0x14960u, 0x15568u, 0x055a0u, 0x0ada0u, 0x0b536u, 0x1b520u, 0x1b240u,
    0x1d2a4u, 0x1a4a0u, 0x1c9aau, 0x14ac0u, 0x056c0u, 0x05ea6u, 0x0daa0u, 0x1d520u, 0x1ea45u,
    0x1d240u, 0x1a4c0u, 0x0a5c3u, 0x14ae0u, 0x15ac8u, 0x06b40u, 0x0daa0u, 0x16d25u, 0x0e920u,
    0x0d260u, 0x15364u, 0x0a560u, 0x14b60u, 0x155c2u, 0x0ad40u, 0x1baa7u, 0x17480u, 0x16920u,
    0x1aa65u, 0x152a0u, 0x0a5a0u, 0x0aba4u, 0x156a0u, 0x17549u, 0x0ba40u, 0x1b4a0u, 0x1d156u,
    0x1a940u, 0x192a0u, 0x153c4u, 0x0aac0u, 0x156a0u, 0x15b42u, 0x0da40u, 0x0eca6u, 0x0e4a0u,
    0x0c960u, 0x0cae5u, 0x19560u, 0x0ab40u, 0x0adc3u, 0x16d20u, 0x1ea4bu, 0x16a40u, 0x164a0u,
    0x1a176u, 0x14960u, 0x09560u, 0x05765u, 0x0b5a0u, 0x16d40u, 0x1b542u, 0x1b240u, 0x1d4a7u,
    0x1a4a0u, 0x14aa0u, 0x149b5u, 0x096c0u, 0x0b6a0u, 0x0da53u, 0x1d920u, 0x1f248u, 0x1d240u,
    0x1a4c0u, 0x0a2d6u, 0x14ae0u, 0x09ac0u, 0x06cb4u, 0x0eaa0u, 0x0e920u, 0x0e963u, 0x0d260u,
    0x15567u, 0x0a560u, 0x14b60u, 0x15745u, 0x0ad40u, 0x16ca0u, 0x17544u, 0x16940u, 0x1b2a8u,
    0x152a0u, 0x0a5a0u, 0x0ada6u, 0x156a0u, 0x0b540u, 0x0baa4u, 0x1b4a0u, 0x1a940u, 0x1c9a3u,
    0x192c0u, 0x199c7u, 0x0aac0u, 0x156a0u, 0x15a55u, 0x0da40u, 0x1d4a0u, 0x0e544u, 0x0c960u,
    0x0d2e8u, 0x09560u, 0x0ab60u, 0x0aad6u, 0x16d40u, 0x0ea40u, 0x172a4u, 0x164a0u, 0x15160u,
    0x149e2u, 0x09560u,
}};

auto leapMonth(std::uint32_t info) -> int
{
    return int(info & 0x0fu);
}

auto leapMonthDays(std::uint32_t info) -> int
{
    return (info & 0x10u) != 0 ? 30 : 29;
}

auto monthDays(std::uint32_t info, int month) -> int
{
    return ((info >> (4 + month)) & 1u) != 0 ? 30 : 29;
}

auto yearDays(std::uint32_t info) -> int
{
    int days = 0;
    for (int month = 1; month <= kMonthCount; ++month)
        days += monthDays(info, month);
    if (leapMonth(info) != 0)
        days += leapMonthDays(info);
    return days;
}

} // namespace

auto isLunarDateSupported(const QDate &date) -> bool
{
    return date.isValid() && date >= QDate(kBaseYear, kBaseMonth, kBaseDay)
           && date <= QDate(kLastYear, 12, 31);
}

auto toLunar(const QDate &date) -> Utils::Result<LunarDate>
{
    if (!isLunarDateSupported(date))
        return std::unexpected(FortuneStrings::tr("Date is outside the supported 1901-2100 range."));

    qsizetype remaining = QDate(kBaseYear, kBaseMonth, kBaseDay).daysTo(date);
    int year = kFirstYear;

    while (year <= kLastYear) {
        const int days = yearDays(kYearInfo[year - kFirstYear]);
        if (remaining < days)
            break;
        remaining -= days;
        ++year;
    }

    if (year > kLastYear)
        return std::unexpected(FortuneStrings::tr("Date is outside the supported 1901-2100 range."));

    const std::uint32_t info = kYearInfo[year - kFirstYear];
    int month = 1;
    int day = 1;
    bool isLeap = false;
    bool found = false;

    for (int candidate = 1; candidate <= kMonthCount && !found; ++candidate) {
        const int length = monthDays(info, candidate);
        if (remaining < length) {
            month = candidate;
            day = int(remaining) + 1;
            found = true;
            break;
        }
        remaining -= length;

        if (leapMonth(info) == candidate) {
            const int leapLength = leapMonthDays(info);
            if (remaining < leapLength) {
                month = candidate;
                day = int(remaining) + 1;
                isLeap = true;
                found = true;
                break;
            }
            remaining -= leapLength;
        }
    }

    if (!found)
        return std::unexpected(FortuneStrings::tr("Lunar conversion failed."));

    const int branch = ((year - 4) % 12 + 12) % 12 + 1;
    return LunarDate{year, month, day, isLeap, branch};
}

} // namespace Fortune
