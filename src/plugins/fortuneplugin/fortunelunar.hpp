#pragma once

#include <utils/result.hpp>

#include <QDate>

namespace Fortune {

struct LunarDate
{
    int year = 1901;
    int month = 1;
    int day = 1;
    bool isLeapMonth = false;
    // Earthly branch of the lunar year: Zi = 1 ... Hai = 12.
    int yearBranch = 1;
};

// Supported input range: 1901-02-19 .. 2100-12-31.
[[nodiscard]] auto toLunar(const QDate &date) -> Utils::Result<LunarDate>;
[[nodiscard]] auto isLunarDateSupported(const QDate &date) -> bool;

} // namespace Fortune
