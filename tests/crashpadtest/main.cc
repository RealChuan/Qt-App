#include <dump/crashpad.hpp>

#include <QCoreApplication>
#include <QDir>

void crash()
{
    int *p = nullptr;
    *p = 1;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QDir dir(a.applicationDirPath());
    if (!dir.exists("crashpad")) {
        dir.mkdir("crashpad");
    }
    auto crashPath = dir.filePath("crashpad");

    Dump::CrashPad crashpad(crashPath, a.applicationDirPath(), "", true);

    crash();

    return a.exec();
}
