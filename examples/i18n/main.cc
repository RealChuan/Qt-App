#include <QLocale>
#include <QMetaEnum>

auto main(int argc, char *argv[]) -> int
{
    auto metaEnums = QMetaEnum::fromType<QLocale::Language>();
    for (int i = 0; i < metaEnums.keyCount(); ++i) {
        qDebug() << metaEnums.key(i) << metaEnums.value(i);
    }
}
