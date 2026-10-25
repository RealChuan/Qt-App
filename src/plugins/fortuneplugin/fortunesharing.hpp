#pragma once

#include "fortuneengine.hpp"

#include <QImage>
#include <QString>

namespace Fortune {

// Plain text for the clipboard.
[[nodiscard]] auto shareText(const Reading &reading) -> QString;

// Renders a standalone share card (board on the left, text on the right) into
// an image. The card is a real widget laid out by Qt, not a viewport grab.
[[nodiscard]] auto renderShareImage(const Reading &reading) -> QImage;

} // namespace Fortune
