// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qtcassert.hpp"

#include <QByteArray>
#include <QDebug>
#include <QMutex>
#include <QTime>

// clang-format off
// Windows.h must come first: DbgHelp.h uses its types. Sorting would swap them.
#if defined(Q_OS_UNIX)
#include <stdio.h>
#include <signal.h>
#include <execinfo.h>
#elif defined(_MSC_VER)
#ifdef QTCREATOR_PCH_H
#define CALLBACK WINAPI
#define OUT
#define IN
#endif
#include <Windows.h>
#include <DbgHelp.h>
#endif
// clang-format on

namespace Utils {

void dumpBacktrace(int maxdepth)
{
    const int ArraySize = 1000;
    if (maxdepth < 0 || maxdepth > ArraySize)
        maxdepth = ArraySize;
#if defined(Q_OS_UNIX)
    void *bt[ArraySize] = {nullptr};
    int size = backtrace(bt, maxdepth);
    char **lines = backtrace_symbols(bt, size);
    for (int i = 0; i < size; ++i)
        qDebug() << "0x" + QByteArray::number(quintptr(bt[i]), 16) << lines[i];
    free(lines);
#elif defined(_MSC_VER) && defined(HAVE_DBGHELP)
    DWORD machineType;
#if defined(_M_X64)
    machineType = IMAGE_FILE_MACHINE_AMD64;
#elif defined(_M_ARM64)
    machineType = IMAGE_FILE_MACHINE_ARM64;
#else
    return;
#endif
    static QMutex mutex;
    const QMutexLocker locker(&mutex);
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    CONTEXT ctx;
    RtlCaptureContext(&ctx);
    STACKFRAME64 frame{};
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;
#if defined(_M_X64)
    frame.AddrPC.Offset = ctx.Rip;
    frame.AddrStack.Offset = ctx.Rsp;
    frame.AddrFrame.Offset = ctx.Rbp;
#elif defined(_M_ARM64)
    frame.AddrPC.Offset = ctx.Pc;
    frame.AddrStack.Offset = ctx.Sp;
    frame.AddrFrame.Offset = ctx.Fp;
#endif
    // ignore the first two frames those contain only writeAssertLocation and dumpBacktrace
    int depth = -3;

    static bool symbolsInitialized = false;
    if (!symbolsInitialized) {
        SymInitialize(process, NULL, TRUE);
        SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
        symbolsInitialized = true;
    }

    while (StackWalk64(
        machineType,
        process,
        thread,
        &frame,
        &ctx,
        NULL,
        &SymFunctionTableAccess64,
        &SymGetModuleBase64,
        NULL)) {
        if (++depth < 0)
            continue;
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO pSymbol = (PSYMBOL_INFO) buffer;
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        if (!SymFromAddr(process, frame.AddrPC.Offset, &displacement, pSymbol))
            break;

        DWORD symDisplacement = 0;
        IMAGEHLP_LINE64 lineInfo;
        SymSetOptions(SYMOPT_LOAD_LINES);

        QString out = QStringLiteral("  %1: 0x%2 at %3")
                          .arg(depth)
                          .arg(QString::number(pSymbol->Address, 16))
                          .arg(QString::fromLatin1(&pSymbol->Name[0], pSymbol->NameLen));
        if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &symDisplacement, &lineInfo)) {
            out.append(QStringLiteral(" (%1:%2)")
                           .arg(
                               QString::fromLatin1(lineInfo.FileName),
                               QString::number(lineInfo.LineNumber)));
        }
        qDebug().noquote() << out;
        if (depth == maxdepth)
            break;
    }
#endif
}

void writeAssertLocation(const char *msg)
{
    const QByteArray time = QTime::currentTime().toString(Qt::ISODateWithMs).toLatin1();
    static bool goBoom = qEnvironmentVariableIsSet("QTC_FATAL_ASSERTS");
    if (goBoom)
        qFatal("SOFT ASSERT [%s] made fatal: %s", time.data(), msg);
    else
        qDebug("SOFT ASSERT [%s]: %s", time.data(), msg);

    static int maxdepth = qEnvironmentVariableIntValue("QTC_BACKTRACE_MAXDEPTH");
    if (maxdepth != 0)
        dumpBacktrace(maxdepth);
}

} // namespace Utils
