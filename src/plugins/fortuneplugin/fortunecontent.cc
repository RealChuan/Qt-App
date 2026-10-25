#include "fortunecontent.hpp"

#include <QCoreApplication>

#include <algorithm>
#include <array>

namespace Fortune {

namespace {

constexpr std::array<TrigramInfo, 8> kTrigrams{{
    {1,
     QT_TRANSLATE_NOOP("FortuneContent", "Qian"),
     Element::Metal,
     QT_TRANSLATE_NOOP("FortuneContent", "Heaven")},
    {2,
     QT_TRANSLATE_NOOP("FortuneContent", "Dui"),
     Element::Metal,
     QT_TRANSLATE_NOOP("FortuneContent", "Lake")},
    {3,
     QT_TRANSLATE_NOOP("FortuneContent", "Li"),
     Element::Fire,
     QT_TRANSLATE_NOOP("FortuneContent", "Fire")},
    {4,
     QT_TRANSLATE_NOOP("FortuneContent", "Zhen"),
     Element::Wood,
     QT_TRANSLATE_NOOP("FortuneContent", "Thunder")},
    {5,
     QT_TRANSLATE_NOOP("FortuneContent", "Xun"),
     Element::Wood,
     QT_TRANSLATE_NOOP("FortuneContent", "Wind")},
    {6,
     QT_TRANSLATE_NOOP("FortuneContent", "Kan"),
     Element::Water,
     QT_TRANSLATE_NOOP("FortuneContent", "Water")},
    {7,
     QT_TRANSLATE_NOOP("FortuneContent", "Gen"),
     Element::Earth,
     QT_TRANSLATE_NOOP("FortuneContent", "Mountain")},
    {8,
     QT_TRANSLATE_NOOP("FortuneContent", "Kun"),
     Element::Earth,
     QT_TRANSLATE_NOOP("FortuneContent", "Earth")},
}};

// King Wen order. The upper/lower pair is the only lookup key used at runtime.
constexpr std::array<HexagramInfo, 64> kHexagrams{{
    {1,
     QT_TRANSLATE_NOOP("FortuneContent", "The Creative"),
     1,
     1,
     QT_TRANSLATE_NOOP("FortuneContent", "Pure force: begin, but give it a direction.")},
    {2,
     QT_TRANSLATE_NOOP("FortuneContent", "The Receptive"),
     8,
     8,
     QT_TRANSLATE_NOOP("FortuneContent", "Yield and carry; the ground does the work.")},
    {3,
     QT_TRANSLATE_NOOP("FortuneContent", "Difficulty at the Beginning"),
     6,
     4,
     QT_TRANSLATE_NOOP("FortuneContent", "A first step is tangled; sort one thread at a time.")},
    {4,
     QT_TRANSLATE_NOOP("FortuneContent", "Youthful Folly"),
     7,
     6,
     QT_TRANSLATE_NOOP("FortuneContent", "Not knowing is the starting point; ask plainly.")},
    {5,
     QT_TRANSLATE_NOOP("FortuneContent", "Waiting"),
     6,
     1,
     QT_TRANSLATE_NOOP("FortuneContent", "The table is set; eat when the food arrives.")},
    {6,
     QT_TRANSLATE_NOOP("FortuneContent", "Conflict"),
     1,
     6,
     QT_TRANSLATE_NOOP(
         "FortuneContent", "A dispute has no clean winner; stop before the third round.")},
    {7,
     QT_TRANSLATE_NOOP("FortuneContent", "The Army"),
     8,
     6,
     QT_TRANSLATE_NOOP("FortuneContent", "Many hands need one rhythm, not many voices.")},
    {8,
     QT_TRANSLATE_NOOP("FortuneContent", "Holding Together"),
     6,
     8,
     QT_TRANSLATE_NOOP("FortuneContent", "Join the group that already shares your direction.")},
    {9,
     QT_TRANSLATE_NOOP("FortuneContent", "The Taming Power of the Small"),
     5,
     1,
     QT_TRANSLATE_NOOP("FortuneContent", "Small adjustments, repeated, beat one large push.")},
    {10,
     QT_TRANSLATE_NOOP("FortuneContent", "Treading"),
     1,
     2,
     QT_TRANSLATE_NOOP("FortuneContent", "Walk carefully past power; courtesy is the path.")},
    {11,
     QT_TRANSLATE_NOOP("FortuneContent", "Peace"),
     8,
     1,
     QT_TRANSLATE_NOOP("FortuneContent", "The middle holds; use the calm to repair what is loose.")},
    {12,
     QT_TRANSLATE_NOOP("FortuneContent", "Standstill"),
     1,
     8,
     QT_TRANSLATE_NOOP("FortuneContent", "The channels are blocked; keep your shape and wait.")},
    {13,
     QT_TRANSLATE_NOOP("FortuneContent", "Fellowship with Men"),
     1,
     3,
     QT_TRANSLATE_NOOP("FortuneContent", "Shared work, openly stated, is the only bond here.")},
    {14,
     QT_TRANSLATE_NOOP("FortuneContent", "Great Possession"),
     3,
     1,
     QT_TRANSLATE_NOOP("FortuneContent", "Much is held; hold it lightly.")},
    {15,
     QT_TRANSLATE_NOOP("FortuneContent", "Modesty"),
     8,
     7,
     QT_TRANSLATE_NOOP("FortuneContent", "Lower the volume; the work will speak.")},
    {16,
     QT_TRANSLATE_NOOP("FortuneContent", "Enthusiasm"),
     4,
     8,
     QT_TRANSLATE_NOOP("FortuneContent", "Momentum is useful once; do not mistake it for a plan.")},
    {17,
     QT_TRANSLATE_NOOP("FortuneContent", "Following"),
     2,
     4,
     QT_TRANSLATE_NOOP("FortuneContent", "Follow what is already moving, not what you wish were.")},
    {18,
     QT_TRANSLATE_NOOP("FortuneContent", "Work on the Decayed"),
     7,
     5,
     QT_TRANSLATE_NOOP("FortuneContent", "Repair the old thing before starting another.")},
    {19,
     QT_TRANSLATE_NOOP("FortuneContent", "Approach"),
     8,
     2,
     QT_TRANSLATE_NOOP("FortuneContent", "Nearness is offered; arrive with something to give.")},
    {20,
     QT_TRANSLATE_NOOP("FortuneContent", "Contemplation"),
     5,
     8,
     QT_TRANSLATE_NOOP("FortuneContent", "Look at the whole pattern before joining it.")},
    {21,
     QT_TRANSLATE_NOOP("FortuneContent", "Biting Through"),
     3,
     4,
     QT_TRANSLATE_NOOP("FortuneContent", "A clear obstacle needs one clean decision.")},
    {22,
     QT_TRANSLATE_NOOP("FortuneContent", "Grace"),
     7,
     3,
     QT_TRANSLATE_NOOP(
         "FortuneContent", "Surface matters, but do not mistake the polish for the thing.")},
    {23,
     QT_TRANSLATE_NOOP("FortuneContent", "Splitting Apart"),
     7,
     8,
     QT_TRANSLATE_NOOP("FortuneContent", "Do not build on a surface that is already peeling.")},
    {24,
     QT_TRANSLATE_NOOP("FortuneContent", "Return"),
     8,
     4,
     QT_TRANSLATE_NOOP("FortuneContent", "Turn back early; the road is still short.")},
    {25,
     QT_TRANSLATE_NOOP("FortuneContent", "Innocence"),
     1,
     4,
     QT_TRANSLATE_NOOP("FortuneContent", "Act without calculation, but keep the facts.")},
    {26,
     QT_TRANSLATE_NOOP("FortuneContent", "The Taming Power of the Great"),
     7,
     1,
     QT_TRANSLATE_NOOP("FortuneContent", "Strength is stored, not spent; train quietly.")},
    {27,
     QT_TRANSLATE_NOOP("FortuneContent", "The Corners of the Mouth"),
     7,
     4,
     QT_TRANSLATE_NOOP("FortuneContent", "What enters the mouth: choose it deliberately.")},
    {28,
     QT_TRANSLATE_NOOP("FortuneContent", "Preponderance of the Great"),
     2,
     5,
     QT_TRANSLATE_NOOP("FortuneContent", "The load is too heavy; change the beam, not the bearer.")},
    {29,
     QT_TRANSLATE_NOOP("FortuneContent", "The Abysmal"),
     6,
     6,
     QT_TRANSLATE_NOOP("FortuneContent", "Go through, not around; keep one hand on the wall.")},
    {30,
     QT_TRANSLATE_NOOP("FortuneContent", "The Clinging"),
     3,
     3,
     QT_TRANSLATE_NOOP("FortuneContent", "Attach to what gives light, and keep a way out.")},
    {31,
     QT_TRANSLATE_NOOP("FortuneContent", "Influence"),
     2,
     7,
     QT_TRANSLATE_NOOP("FortuneContent", "A small signal reaches far; mean it.")},
    {32,
     QT_TRANSLATE_NOOP("FortuneContent", "Duration"),
     4,
     5,
     QT_TRANSLATE_NOOP("FortuneContent", "Continue at a pace you can keep.")},
    {33,
     QT_TRANSLATE_NOOP("FortuneContent", "Retreat"),
     1,
     7,
     QT_TRANSLATE_NOOP("FortuneContent", "Withdraw in good order; it is not defeat.")},
    {34,
     QT_TRANSLATE_NOOP("FortuneContent", "The Power of the Great"),
     4,
     1,
     QT_TRANSLATE_NOOP("FortuneContent", "Force is available; spend it only where it belongs.")},
    {35,
     QT_TRANSLATE_NOOP("FortuneContent", "Progress"),
     3,
     8,
     QT_TRANSLATE_NOOP("FortuneContent", "Move into the light; bring others with you.")},
    {36,
     QT_TRANSLATE_NOOP("FortuneContent", "The Darkening of the Light"),
     8,
     3,
     QT_TRANSLATE_NOOP("FortuneContent", "Keep the light inside; visibility is not safety.")},
    {37,
     QT_TRANSLATE_NOOP("FortuneContent", "The Family"),
     5,
     3,
     QT_TRANSLATE_NOOP("FortuneContent", "Roles stated plainly keep the house quiet.")},
    {38,
     QT_TRANSLATE_NOOP("FortuneContent", "Opposition"),
     3,
     2,
     QT_TRANSLATE_NOOP("FortuneContent", "Two truths at once; look for the small shared piece.")},
    {39,
     QT_TRANSLATE_NOOP("FortuneContent", "Obstruction"),
     6,
     7,
     QT_TRANSLATE_NOOP("FortuneContent", "The way is blocked; turn to the side and ask for help.")},
    {40,
     QT_TRANSLATE_NOOP("FortuneContent", "Deliverance"),
     4,
     6,
     QT_TRANSLATE_NOOP("FortuneContent", "The knot loosens; finish the release, do not reopen it.")},
    {41,
     QT_TRANSLATE_NOOP("FortuneContent", "Decrease"),
     7,
     2,
     QT_TRANSLATE_NOOP("FortuneContent", "Take from the excess, add to the base.")},
    {42,
     QT_TRANSLATE_NOOP("FortuneContent", "Increase"),
     5,
     4,
     QT_TRANSLATE_NOOP("FortuneContent", "Give where it travels; gain follows the giving.")},
    {43,
     QT_TRANSLATE_NOOP("FortuneContent", "Break-through"),
     2,
     1,
     QT_TRANSLATE_NOOP("FortuneContent", "Say it once, clearly, and do not humiliate.")},
    {44,
     QT_TRANSLATE_NOOP("FortuneContent", "Coming to Meet"),
     1,
     5,
     QT_TRANSLATE_NOOP("FortuneContent", "A strong encounter; check what it wants from you.")},
    {45,
     QT_TRANSLATE_NOOP("FortuneContent", "Gathering Together"),
     2,
     8,
     QT_TRANSLATE_NOOP("FortuneContent", "People gather around a real need; name it.")},
    {46,
     QT_TRANSLATE_NOOP("FortuneContent", "Pushing Upward"),
     8,
     5,
     QT_TRANSLATE_NOOP("FortuneContent", "Rise step by step; do not skip the root.")},
    {47,
     QT_TRANSLATE_NOOP("FortuneContent", "Oppression"),
     2,
     6,
     QT_TRANSLATE_NOOP("FortuneContent", "Speech is thin here; let the work answer.")},
    {48,
     QT_TRANSLATE_NOOP("FortuneContent", "The Well"),
     6,
     5,
     QT_TRANSLATE_NOOP("FortuneContent", "The source is shared; maintain the rope and the bucket.")},
    {49,
     QT_TRANSLATE_NOOP("FortuneContent", "Revolution"),
     2,
     3,
     QT_TRANSLATE_NOOP(
         "FortuneContent", "Change the frame only when the old one has stopped holding.")},
    {50,
     QT_TRANSLATE_NOOP("FortuneContent", "The Cauldron"),
     3,
     5,
     QT_TRANSLATE_NOOP("FortuneContent", "Transform what is raw into something that can be shared.")},
    {51,
     QT_TRANSLATE_NOOP("FortuneContent", "The Arousing"),
     4,
     4,
     QT_TRANSLATE_NOOP("FortuneContent", "Shock clears the air; put things back in order afterward.")},
    {52,
     QT_TRANSLATE_NOOP("FortuneContent", "Keeping Still"),
     7,
     7,
     QT_TRANSLATE_NOOP("FortuneContent", "Stop at the right place, not at the first convenient one.")},
    {53,
     QT_TRANSLATE_NOOP("FortuneContent", "Development"),
     5,
     7,
     QT_TRANSLATE_NOOP("FortuneContent", "Grow in order: root, trunk, branch.")},
    {54,
     QT_TRANSLATE_NOOP("FortuneContent", "The Marrying Maiden"),
     4,
     2,
     QT_TRANSLATE_NOOP("FortuneContent", "A position held without consent will not last.")},
    {55,
     QT_TRANSLATE_NOOP("FortuneContent", "Abundance"),
     4,
     3,
     QT_TRANSLATE_NOOP("FortuneContent", "Peak light: see it, use it, and prepare for the dimming.")},
    {56,
     QT_TRANSLATE_NOOP("FortuneContent", "The Wanderer"),
     3,
     7,
     QT_TRANSLATE_NOOP("FortuneContent", "Keep the bag light and the conduct clean.")},
    {57,
     QT_TRANSLATE_NOOP("FortuneContent", "The Gentle"),
     5,
     5,
     QT_TRANSLATE_NOOP("FortuneContent", "Persistence in small doses; enter by the gap.")},
    {58,
     QT_TRANSLATE_NOOP("FortuneContent", "The Joyous"),
     2,
     2,
     QT_TRANSLATE_NOOP("FortuneContent", "Pleasure shared is useful; pleasure borrowed is not.")},
    {59,
     QT_TRANSLATE_NOOP("FortuneContent", "Dispersion"),
     5,
     6,
     QT_TRANSLATE_NOOP(
         "FortuneContent", "Dissolve the block, then return the pieces to their places.")},
    {60,
     QT_TRANSLATE_NOOP("FortuneContent", "Limitation"),
     6,
     2,
     QT_TRANSLATE_NOOP("FortuneContent", "A clear limit is a form of care.")},
    {61,
     QT_TRANSLATE_NOOP("FortuneContent", "Inner Truth"),
     5,
     2,
     QT_TRANSLATE_NOOP(
         "FortuneContent", "Words without weight do not move anyone; mean what you say.")},
    {62,
     QT_TRANSLATE_NOOP("FortuneContent", "Preponderance of the Small"),
     4,
     7,
     QT_TRANSLATE_NOOP(
         "FortuneContent", "Stay small in the detail; the grand gesture is out of scale.")},
    {63,
     QT_TRANSLATE_NOOP("FortuneContent", "After Completion"),
     6,
     3,
     QT_TRANSLATE_NOOP("FortuneContent", "Done is dangerous; check the seams before declaring it.")},
    {64,
     QT_TRANSLATE_NOOP("FortuneContent", "Before Completion"),
     3,
     6,
     QT_TRANSLATE_NOOP("FortuneContent", "Almost through; keep the order of steps until the end.")},
}};

constexpr std::array<int, 2> kMetalNumbers{4, 9};
constexpr std::array<int, 2> kWoodNumbers{3, 8};
constexpr std::array<int, 2> kWaterNumbers{1, 6};
constexpr std::array<int, 2> kFireNumbers{2, 7};
constexpr std::array<int, 2> kEarthNumbers{5, 10};

constexpr std::array<const char *, 6> kDoGreat{
    QT_TRANSLATE_NOOP("FortuneContent", "start the thing you have been circling"),
    QT_TRANSLATE_NOOP("FortuneContent", "make the ask you have rehearsed"),
    QT_TRANSLATE_NOOP("FortuneContent", "share the good news in person"),
    QT_TRANSLATE_NOOP("FortuneContent", "take the opening while it is open"),
    QT_TRANSLATE_NOOP("FortuneContent", "put your name on the work"),
    QT_TRANSLATE_NOOP("FortuneContent", "say yes to the useful invitation"),
};

constexpr std::array<const char *, 6> kAvoidGreat{
    QT_TRANSLATE_NOOP("FortuneContent", "spending the whole surplus at once"),
    QT_TRANSLATE_NOOP("FortuneContent", "confusing luck with a plan"),
    QT_TRANSLATE_NOOP("FortuneContent", "promising more than the week allows"),
    QT_TRANSLATE_NOOP("FortuneContent", "letting a small win go unrecorded"),
    QT_TRANSLATE_NOOP("FortuneContent", "skipping the handover"),
    QT_TRANSLATE_NOOP("FortuneContent", "testing the goodwill twice"),
};

constexpr std::array<const char *, 6> kDoGood{
    QT_TRANSLATE_NOOP("FortuneContent", "keep the current pace"),
    QT_TRANSLATE_NOOP("FortuneContent", "finish one open thread"),
    QT_TRANSLATE_NOOP("FortuneContent", "answer the message you have delayed"),
    QT_TRANSLATE_NOOP("FortuneContent", "make the quiet improvement"),
    QT_TRANSLATE_NOOP("FortuneContent", "check in with the person who helped"),
    QT_TRANSLATE_NOOP("FortuneContent", "write down what worked"),
};

constexpr std::array<const char *, 6> kAvoidGood{
    QT_TRANSLATE_NOOP("FortuneContent", "restarting from zero"),
    QT_TRANSLATE_NOOP("FortuneContent", "adding a second goal"),
    QT_TRANSLATE_NOOP("FortuneContent", "reading too much into a calm day"),
    QT_TRANSLATE_NOOP("FortuneContent", "moving a boundary without saying so"),
    QT_TRANSLATE_NOOP("FortuneContent", "saving the easy task for later"),
    QT_TRANSLATE_NOOP("FortuneContent", "treating silence as agreement"),
};

constexpr std::array<const char *, 6> kDoFair{
    QT_TRANSLATE_NOOP("FortuneContent", "take the initiative on the small thing"),
    QT_TRANSLATE_NOOP("FortuneContent", "ask for a clear deadline"),
    QT_TRANSLATE_NOOP("FortuneContent", "keep the proof of the agreement"),
    QT_TRANSLATE_NOOP("FortuneContent", "do the boring part first"),
    QT_TRANSLATE_NOOP("FortuneContent", "leave earlier than usual"),
    QT_TRANSLATE_NOOP("FortuneContent", "say what you can actually deliver"),
};

constexpr std::array<const char *, 6> kAvoidFair{
    QT_TRANSLATE_NOOP("FortuneContent", "picking a fight to test the ground"),
    QT_TRANSLATE_NOOP("FortuneContent", "lending what you may need back"),
    QT_TRANSLATE_NOOP("FortuneContent", "assuming the other side knows"),
    QT_TRANSLATE_NOOP("FortuneContent", "changing the plan mid-sentence"),
    QT_TRANSLATE_NOOP("FortuneContent", "arguing with a closed door"),
    QT_TRANSLATE_NOOP("FortuneContent", "spending energy on being right"),
};

constexpr std::array<const char *, 6> kDoCaution{
    QT_TRANSLATE_NOOP("FortuneContent", "reduce the scope to what fits"),
    QT_TRANSLATE_NOOP("FortuneContent", "keep a written trail"),
    QT_TRANSLATE_NOOP("FortuneContent", "rest before the second push"),
    QT_TRANSLATE_NOOP("FortuneContent", "ask one more question"),
    QT_TRANSLATE_NOOP("FortuneContent", "protect the margin you already have"),
    QT_TRANSLATE_NOOP("FortuneContent", "do the maintenance, not the launch"),
};

constexpr std::array<const char *, 6> kAvoidCaution{
    QT_TRANSLATE_NOOP("FortuneContent", "signing on the first look"),
    QT_TRANSLATE_NOOP("FortuneContent", "spending tomorrow's energy today"),
    QT_TRANSLATE_NOOP("FortuneContent", "giving the first number"),
    QT_TRANSLATE_NOOP("FortuneContent", "letting a small leak run"),
    QT_TRANSLATE_NOOP("FortuneContent", "taking the loudest option"),
    QT_TRANSLATE_NOOP("FortuneContent", "explaining yourself twice"),
};

constexpr std::array<const char *, 6> kDoUnfavourable{
    QT_TRANSLATE_NOOP("FortuneContent", "stop and check the premise"),
    QT_TRANSLATE_NOOP("FortuneContent", "keep the answer short"),
    QT_TRANSLATE_NOOP("FortuneContent", "move the meeting or the deadline"),
    QT_TRANSLATE_NOOP("FortuneContent", "secure the basics first"),
    QT_TRANSLATE_NOOP("FortuneContent", "ask for time you do not have to justify"),
    QT_TRANSLATE_NOOP("FortuneContent", "let the other side go first"),
};

constexpr std::array<const char *, 6> kAvoidUnfavourable{
    QT_TRANSLATE_NOOP("FortuneContent", "forcing the opening"),
    QT_TRANSLATE_NOOP("FortuneContent", "borrowing to cover a gap"),
    QT_TRANSLATE_NOOP("FortuneContent", "sending the message at peak temper"),
    QT_TRANSLATE_NOOP("FortuneContent", "betting the reserve"),
    QT_TRANSLATE_NOOP("FortuneContent", "treating a warning as noise"),
    QT_TRANSLATE_NOOP("FortuneContent", "making a permanent choice today"),
};

} // namespace

auto trigrams() -> std::span<const TrigramInfo>
{
    return kTrigrams;
}

auto trigram(int id) -> const TrigramInfo &
{
    return kTrigrams[std::clamp(id, 1, int(kTrigrams.size())) - 1];
}

auto trigramName(int id) -> QString
{
    return QCoreApplication::translate("FortuneContent", trigram(id).name);
}

auto trigramImage(int id) -> QString
{
    return QCoreApplication::translate("FortuneContent", trigram(id).image);
}

auto hexagrams() -> std::span<const HexagramInfo>
{
    return kHexagrams;
}

auto hexagram(int number) -> const HexagramInfo &
{
    return kHexagrams[std::clamp(number, 1, int(kHexagrams.size())) - 1];
}

auto hexagramName(int number) -> QString
{
    return QCoreApplication::translate("FortuneContent", hexagram(number).name);
}

auto hexagramJudgment(int number) -> QString
{
    return QCoreApplication::translate("FortuneContent", hexagram(number).judgment);
}

auto hexagramFor(int upper, int lower) -> int
{
    for (const HexagramInfo &info : kHexagrams) {
        if (info.upper == upper && info.lower == lower)
            return info.number;
    }
    return 1;
}

auto trigramLineMask(int id) -> int
{
    switch (id) {
    case 1:
        return 0b111;
    case 2:
        return 0b011;
    case 3:
        return 0b101;
    case 4:
        return 0b001;
    case 5:
        return 0b110;
    case 6:
        return 0b010;
    case 7:
        return 0b100;
    case 8:
        return 0b000;
    }
    return 0b000;
}

auto trigramFromLineMask(int mask) -> int
{
    switch (mask & 0b111) {
    case 0b111:
        return 1;
    case 0b011:
        return 2;
    case 0b101:
        return 3;
    case 0b001:
        return 4;
    case 0b110:
        return 5;
    case 0b010:
        return 6;
    case 0b100:
        return 7;
    default:
        return 8;
    }
}

auto elementName(Element element) -> QString
{
    switch (element) {
    case Element::Metal:
        return QCoreApplication::translate("FortuneContent", "Metal");
    case Element::Wood:
        return QCoreApplication::translate("FortuneContent", "Wood");
    case Element::Water:
        return QCoreApplication::translate("FortuneContent", "Water");
    case Element::Fire:
        return QCoreApplication::translate("FortuneContent", "Fire");
    case Element::Earth:
        return QCoreApplication::translate("FortuneContent", "Earth");
    }
    return QCoreApplication::translate("FortuneContent", "Earth");
}

auto elementColorHex(Element element) -> const char *
{
    switch (element) {
    case Element::Metal:
        return "#C9CDD6";
    case Element::Wood:
        return "#3E8E5A";
    case Element::Water:
        return "#2F6F9F";
    case Element::Fire:
        return "#C7503A";
    case Element::Earth:
        return "#B08A3E";
    }
    return "#B08A3E";
}

auto elementLuckyNumbers(Element element) -> std::span<const int>
{
    switch (element) {
    case Element::Metal:
        return kMetalNumbers;
    case Element::Wood:
        return kWoodNumbers;
    case Element::Water:
        return kWaterNumbers;
    case Element::Fire:
        return kFireNumbers;
    case Element::Earth:
        return kEarthNumbers;
    }
    return kEarthNumbers;
}

auto gradeName(Grade grade) -> QString
{
    switch (grade) {
    case Grade::Unfavourable:
        return QCoreApplication::translate("FortuneContent", "Unfavourable");
    case Grade::Caution:
        return QCoreApplication::translate("FortuneContent", "Caution");
    case Grade::Fair:
        return QCoreApplication::translate("FortuneContent", "Fair");
    case Grade::Good:
        return QCoreApplication::translate("FortuneContent", "Good");
    case Grade::GreatFortune:
        return QCoreApplication::translate("FortuneContent", "Great fortune");
    }
    return QCoreApplication::translate("FortuneContent", "Fair");
}

auto doAdvice(Grade grade) -> std::span<const char *const>
{
    switch (grade) {
    case Grade::GreatFortune:
        return kDoGreat;
    case Grade::Good:
        return kDoGood;
    case Grade::Fair:
        return kDoFair;
    case Grade::Caution:
        return kDoCaution;
    case Grade::Unfavourable:
        return kDoUnfavourable;
    }
    return kDoFair;
}

auto avoidAdvice(Grade grade) -> std::span<const char *const>
{
    switch (grade) {
    case Grade::GreatFortune:
        return kAvoidGreat;
    case Grade::Good:
        return kAvoidGood;
    case Grade::Fair:
        return kAvoidFair;
    case Grade::Caution:
        return kAvoidCaution;
    case Grade::Unfavourable:
        return kAvoidUnfavourable;
    }
    return kAvoidFair;
}

} // namespace Fortune
