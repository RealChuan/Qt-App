#pragma once

#include <QString>

#include <span>

namespace Fortune {

enum class Element { Metal, Wood, Water, Fire, Earth };

// Ordered from worst to best: the birth element bonus simply moves one step up.
enum class Grade { Unfavourable, Caution, Fair, Good, GreatFortune };

struct TrigramInfo
{
    int id = 1;
    const char *name = "";
    Element element = Element::Earth;
    const char *image = "";
};

struct HexagramInfo
{
    int number = 1;
    const char *name = "";
    int upper = 1;
    int lower = 1;
    const char *judgment = "";
};

[[nodiscard]] auto trigrams() -> std::span<const TrigramInfo>;
[[nodiscard]] auto trigram(int id) -> const TrigramInfo &;
[[nodiscard]] auto trigramName(int id) -> QString;
[[nodiscard]] auto trigramImage(int id) -> QString;
[[nodiscard]] auto hexagrams() -> std::span<const HexagramInfo>;
[[nodiscard]] auto hexagram(int number) -> const HexagramInfo &;
[[nodiscard]] auto hexagramName(int number) -> QString;
[[nodiscard]] auto hexagramJudgment(int number) -> QString;
[[nodiscard]] auto hexagramFor(int upper, int lower) -> int;

// Bit 0 is the bottom line, 1 means yang. The upper trigram occupies bits 3-5.
[[nodiscard]] auto trigramLineMask(int id) -> int;
[[nodiscard]] auto trigramFromLineMask(int mask) -> int;

[[nodiscard]] auto elementName(Element element) -> QString;
[[nodiscard]] auto elementColorHex(Element element) -> const char *;
[[nodiscard]] auto elementLuckyNumbers(Element element) -> std::span<const int>;

[[nodiscard]] auto gradeName(Grade grade) -> QString;
[[nodiscard]] auto doAdvice(Grade grade) -> std::span<const char *const>;
[[nodiscard]] auto avoidAdvice(Grade grade) -> std::span<const char *const>;

} // namespace Fortune
