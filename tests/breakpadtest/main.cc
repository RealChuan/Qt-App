#include <dump/breakpad.hpp>

#include <QCoreApplication>
#include <QDir>

void crash()
{
    int *p = nullptr;
    *p = 1;
}

auto main(int argc, char *argv[]) -> int
{
    QCoreApplication a(argc, argv);

    QDir dir(a.applicationDirPath());
    if (!dir.exists("breakpad")) {
        dir.mkdir("breakpad");
    }
    auto crashPath = dir.filePath("breakpad");

    Dump::Breakpad breakpad(crashPath.toStdString());

    crash();

    return a.exec();
}
