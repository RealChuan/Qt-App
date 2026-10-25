#pragma once

#include <QCoreApplication>

namespace Fortune {

// Translation context for free functions (share text, share card and engine
// errors) that cannot use QObject::tr.
class FortuneStrings
{
    Q_DECLARE_TR_FUNCTIONS(FortuneStrings)

public:
    FortuneStrings() = delete;
};

} // namespace Fortune
