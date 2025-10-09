// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "utilsicons.h"

namespace Utils {
namespace Icons {

const Icon EDIT_CLEAR({{":/utils/images/editclear.png", "ff909090"}}, Icon::Tint);
const Icon OK({{":/utils/images/ok.png", "ff6da838"}}, Icon::Tint);
const Icon NOTLOADED({{":/utils/images/notloaded.png", "ffdf4f4f"}}, Icon::Tint);
const Icon BROKEN({{":/utils/images/broken.png", "ffdf4f4f"}}, Icon::Tint);
const Icon CRITICAL({{":/utils/images/warningfill.png", "ffffffff"},
                     {":/utils/images/error.png", "ffdf4f4f"}},
                    Icon::Tint);

const Icon WARNING({{":/utils/images/warningfill.png", "ffffffff"},
                    {":/utils/images/warning.png", "ffecbc1c"}},
                   Icon::Tint);
const Icon INFO({{":/utils/images/warningfill.png", "ffffffff"},
                 {":/utils/images/info.png", "ff3099dc"}},
                Icon::Tint);

#define MAKE_ENTRY(ICON) {#ICON, ICON}
static QHash<QString, Icon> s_nameToIcon = {MAKE_ENTRY(BROKEN),
                                            MAKE_ENTRY(CRITICAL),
                                            MAKE_ENTRY(EDIT_CLEAR),
                                            MAKE_ENTRY(INFO),
                                            MAKE_ENTRY(NOTLOADED),
                                            MAKE_ENTRY(OK),
                                            MAKE_ENTRY(WARNING)};

std::optional<Icon> fromString(const QString &name)
{
    auto it = s_nameToIcon.find(name);
    if (it != s_nameToIcon.end())
        return it.value();
    return std::nullopt;
}

} // namespace Icons

} // namespace Utils
