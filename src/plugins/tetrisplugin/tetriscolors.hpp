#pragma once

#include "tetrisgame.hpp"

#include <QColor>
#include <QGuiApplication>
#include <QPalette>

#include <array>
#include <cstddef>

namespace Tetris {

// 七种方块沿用约定俗成的色相，明度与饱和度按深浅主题调：
// 浅色底上压暗一点才看得清，深色底上提亮才不糊成一片。
inline auto pieceColor(PieceType type) -> QColor
{
    static constexpr std::array<int, kPieceCount> kHueDegrees{
        180, // I 青
        220, // J 蓝
        30,  // L 橙
        50,  // O 黄
        120, // S 绿
        280, // T 紫
        0,   // Z 红
    };

    const QPalette palette = QGuiApplication::palette();
    const bool isDark = palette.color(QPalette::Window).lightnessF() < 0.5;
    const float hue = static_cast<float>(kHueDegrees.at(static_cast<std::size_t>(type))) / 360.0F;
    return QColor::fromHsvF(hue, isDark ? 0.62F : 0.78F, isDark ? 0.95F : 0.82F);
}

} // namespace Tetris
