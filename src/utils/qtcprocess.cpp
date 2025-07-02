// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qtcprocess.h"

#include "algorithm.h"
#include "environment.h"
#include "guard.h"
#include "hostosinfo.h"
#include "processhelper.h"
#include "processinterface.h"
#include "processreaper.h"
#include "stringutils.h"
// #include "terminalhooks.h"
#include "textcodec.h"
#include "threadutils.h"
#include "utilstr.h"

// #include <iptyprocess.h>
// #include <ptyqt.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QMutex>
#include <QScopeGuard>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>

#ifdef QT_GUI_LIB
// qmlpuppet does not use that.
#include <QGuiApplication>
#include <QMessageBox>
#include <QStringConverterBase>
#endif

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>

using namespace Utils::Internal;

using namespace std::chrono;

namespace Utils {
namespace Internal {

const char QTC_PROCESS_BLOCKING_TYPE[] = "__BLOCKING_TYPE__";
const char QTC_PROCESS_NUMBER[] = "__NUMBER__";
const char QTC_PROCESS_STARTTIME[] = "__STARTTIME__";

static bool isGuiEnabled()
{
    static bool isGuiApp = qobject_cast<QGuiApplication *>(qApp);
    return isGuiApp && isMainThread();
}

static bool isMeasuring()
{
    static const bool measuring = qtcEnvironmentVariableIsSet("QTC_MEASURE_PROCESS");
    return measuring;
}

class MeasureAndRun
{
public:
    MeasureAndRun(const char *functionName)
        : m_functionName(functionName)
    {}
    template<typename Function, typename... Args>
    std::invoke_result_t<Function, Args...> measureAndRun(Function &&function, Args &&...args)
    {
        if (!isMeasuring())
            return std::invoke(std::forward<Function>(function), std::forward<Args>(args)...);
        QElapsedTimer timer;
        timer.start();
        const QScopeGuard cleanup([this, &timer] {
            const qint64 currentNsecs = timer.nsecsElapsed();
            const bool mainThread = isMainThread();
            const int hitThisAll = m_hitThisAll.fetch_add(1) + 1;
            const int hitAllAll = m_hitAllAll.fetch_add(1) + 1;
            const int hitThisMain = mainThread ? m_hitThisMain.fetch_add(1) + 1
                                               : m_hitThisMain.load();
            const int hitAllMain = mainThread ? m_hitAllMain.fetch_add(1) + 1 : m_hitAllMain.load();
            const qint64 totalThisAll = toMs(m_totalThisAll.fetch_add(currentNsecs) + currentNsecs);
            const qint64 totalAllAll = toMs(m_totalAllAll.fetch_add(currentNsecs) + currentNsecs);
            const qint64 totalThisMain = toMs(mainThread ? m_totalThisMain.fetch_add(currentNsecs)
                                                               + currentNsecs
                                                         : m_totalThisMain.load());
            const qint64 totalAllMain = toMs(mainThread ? m_totalAllMain.fetch_add(currentNsecs)
                                                              + currentNsecs
                                                        : m_totalAllMain.load());
            printMeasurement(QLatin1String(m_functionName),
                             hitThisAll,
                             toMs(currentNsecs),
                             totalThisAll,
                             hitAllAll,
                             totalAllAll,
                             mainThread,
                             hitThisMain,
                             totalThisMain,
                             hitAllMain,
                             totalAllMain);
        });
        return std::invoke(std::forward<Function>(function), std::forward<Args>(args)...);
    }

private:
    static void printHeader()
    {
        // [function/thread]: function:(T)his|(A)ll, thread:(M)ain|(A)ll
        qDebug() << "+----------------+-------+---------+----------+-------+----------+---------+--"
                    "-----+----------+-------+----------+";
        qDebug() << "| [Function/Thread] = [(T|A)/(M|A)], where: (T)his function, (A)ll functions "
                    "/ threads, (M)ain thread            |";
        qDebug() << "+----------------+-------+---------+----------+-------+----------+---------+--"
                    "-----+----------+-------+----------+";
        qDebug() << "|              1 |     2 |       3 |        4 |     5 |        6 |       7 |  "
                    "   8 |        9 |    10 |       11 |";
        qDebug() << "|                | [T/A] |   [T/A] |    [T/A] | [A/A] |    [A/A] |         | "
                    "[T/M] |    [T/M] | [A/M] |    [A/M] |";
        qDebug() << "|       Function |   Hit | Current |    Total |   Hit |    Total | Current |  "
                    " Hit |    Total |   Hit |    Total |";
        qDebug() << "|           Name | Count |  Measu- |   Measu- | Count |   Measu- | is Main | "
                    "Count |   Measu- | Count |   Measu- |";
        qDebug() << "|                |       |  rement |   rement |       |   rement |  Thread |  "
                    "     |   rement |       |   rement |";
        qDebug() << "+----------------+-------+---------+----------+-------+----------+---------+--"
                    "-----+----------+-------+----------+";
    }
    static void printMeasurement(const QString &functionName,
                                 int hitThisAll,
                                 int currentNsecs,
                                 int totalThisAll,
                                 int hitAllAll,
                                 int totalAllAll,
                                 bool isMainThread,
                                 int hitThisMain,
                                 int totalThisMain,
                                 int hitAllMain,
                                 int totalAllMain)
    {
        static const int repeatHeaderLineCount = 25;
        if (s_lineCounter.fetch_add(1) % repeatHeaderLineCount == 0)
            printHeader();

        const QString &functionNameField = QString("%1").arg(functionName, 14);
        const QString &hitThisAllField = formatField(hitThisAll, 5);
        const QString &currentNsecsField = formatField(currentNsecs, 7, " ms");
        const QString &totalThisAllField = formatField(totalThisAll, 8, " ms");
        const QString &hitAllAllField = formatField(hitAllAll, 5);
        const QString &totalAllAllField = formatField(totalAllAll, 8, " ms");
        const QString &mainThreadField = isMainThread ? QString("%1").arg("yes", 7)
                                                      : QString("%1").arg("no", 7);
        const QString &hitThisMainField = formatField(hitThisMain, 5);
        const QString &totalThisMainField = formatField(totalThisMain, 8, " ms");
        const QString &hitAllMainField = formatField(hitAllMain, 5);
        const QString &totalAllMainField = formatField(totalAllMain, 8, " ms");

        const QString &totalString
            = QString("| %1 | %2 | %3 | %4 | %5 | %6 | %7 | %8 | %9 | %10 | %11 |")
                  .arg(functionNameField,
                       hitThisAllField,
                       currentNsecsField,
                       totalThisAllField,
                       hitAllAllField,
                       totalAllAllField,
                       mainThreadField,
                       hitThisMainField,
                       totalThisMainField,
                       hitAllMainField,
                       totalAllMainField);
        qDebug("%s", qPrintable(totalString));
    }
    static QString formatField(int number, int fieldWidth, const QString &suffix = {})
    {
        return QString("%1%2").arg(number, fieldWidth - suffix.size()).arg(suffix);
    }

    static int toMs(quint64 nsesc) // nanoseconds to miliseconds
    {
        static const int halfMillion = 500000;
        static const int million = 2 * halfMillion;
        return int((nsesc + halfMillion) / million);
    }

    const char *const m_functionName;
    std::atomic_int m_hitThisAll = 0;
    std::atomic_int m_hitThisMain = 0;
    std::atomic_int64_t m_totalThisAll = 0;
    std::atomic_int64_t m_totalThisMain = 0;
    static std::atomic_int m_hitAllAll;
    static std::atomic_int m_hitAllMain;
    static std::atomic_int64_t m_totalAllAll;
    static std::atomic_int64_t m_totalAllMain;
    static std::atomic_int s_lineCounter;
};

std::atomic_int MeasureAndRun::m_hitAllAll = 0;
std::atomic_int MeasureAndRun::m_hitAllMain = 0;
std::atomic_int64_t MeasureAndRun::m_totalAllAll = 0;
std::atomic_int64_t MeasureAndRun::m_totalAllMain = 0;
std::atomic_int MeasureAndRun::s_lineCounter = 0;

static MeasureAndRun s_start = MeasureAndRun("start");
static MeasureAndRun s_waitForStarted = MeasureAndRun("waitForStarted");

static Q_LOGGING_CATEGORY(processLog, "qtc.utils.process", QtWarningMsg) static Q_LOGGING_CATEGORY(
    processStdoutLog,
    "qtc.utils.process.stdout",
    QtWarningMsg) static Q_LOGGING_CATEGORY(processStderrLog,
                                            "qtc.utils.process.stderr",
                                            QtWarningMsg)

    static DeviceProcessHooks s_deviceHooks;

// Data for one channel buffer (stderr/stdout)
class ChannelBuffer
{
public:
    void clearForRun();

    void handleRest();
    void append(const QByteArray &text);

    QByteArray readAllRawData() { return std::exchange(rawData, {}); }

    QString readAllData()
    {
        QString msg = codec.toUnicode(rawData.data(), rawData.size(), &codecState);
        rawData.clear();
        return msg;
    }

    QByteArray rawData;
    QString incompleteLineBuffer; // lines not yet signaled
    TextCodec codec;
    TextCodec::ConverterState codecState;
    std::function<void(const QString &lines)> outputCallback;
    TextChannelMode m_textChannelMode = TextChannelMode::Off;

    bool emitSingleLines = true;
    bool keepRawData = true;
};

class DefaultImpl : public ProcessInterface
{
private:
    void start() final;
    virtual void doDefaultStart(const QString &program, const QStringList &arguments) = 0;
    bool dissolveCommand(QString *program, QStringList *arguments);
    bool ensureProgramExists(const QString &program);
};

void DefaultImpl::start()
{
    QString program;
    QStringList arguments;
    if (!dissolveCommand(&program, &arguments))
        return;
    if (!ensureProgramExists(program))
        return;

    if (m_setup.m_runAsRoot && !HostOsInfo::isWindowsHost()) {
        arguments.prepend(program);
        arguments.prepend("-E");
        arguments.prepend("-A");
        program = "sudo";
    }

    s_start.measureAndRun(&DefaultImpl::doDefaultStart, this, program, arguments);
}

bool DefaultImpl::dissolveCommand(QString *program, QStringList *arguments)
{
    const CommandLine &commandLine = m_setup.m_commandLine;
    const OsType osType = commandLine.executable().osType();

    QString commandString;
    QString processArgs;
    const bool success = ProcessArgs::prepareCommand(commandLine,
                                                     &commandString,
                                                     &processArgs,
                                                     &m_setup.m_environment,
                                                     m_setup.m_workingDirectory);

    if (osType == OsTypeWindows) {
        QString args;
        if (m_setup.m_useCtrlCStub) {
            if (m_setup.m_lowPriority)
                ProcessArgs::addArg(&args, "-nice");
            ProcessArgs::addArg(&args, QDir::toNativeSeparators(commandString));
            commandString = QCoreApplication::applicationDirPath()
                            + QLatin1String("/qtcreator_ctrlc_stub.exe");
        } else if (m_setup.m_lowPriority) {
            m_setup.m_belowNormalPriority = true;
        }
        ProcessArgs::addArgs(&args, processArgs);
        m_setup.m_nativeArguments = args;
        // Note: Arguments set with setNativeArgs will be appended to the ones
        // passed with start() below.
        *arguments = {};
    } else {
        if (!success) {
            const ProcessResultData result = {0,
                                              QProcess::NormalExit,
                                              QProcess::FailedToStart,
                                              Tr::tr("Error in command line.")};
            emit done(result);
            return false;
        }
        *arguments = ProcessArgs::splitArgs(processArgs, osType);
    }
    *program = commandString;
    return true;
}

static FilePath resolve(const FilePath &workingDir, const FilePath &filePath)
{
    if (filePath.isAbsolutePath())
        return filePath;

    const FilePath fromWorkingDir = workingDir.resolvePath(filePath);
    if (fromWorkingDir.exists() && fromWorkingDir.isExecutableFile())
        return fromWorkingDir;
    return filePath.searchInPath();
}

bool DefaultImpl::ensureProgramExists(const QString &program)
{
    const FilePath programFilePath = resolve(m_setup.m_workingDirectory,
                                             FilePath::fromString(program));
    if (programFilePath.exists() && programFilePath.isExecutableFile())
        return true;

    const QString errorString = Tr::tr("The program \"%1\" does not exist or is not executable.")
                                    .arg(program);
    const ProcessResultData result = {0, QProcess::NormalExit, QProcess::FailedToStart, errorString};
    emit done(result);
    return false;
}

// class PtyProcessImpl final : public DefaultImpl
// {
// public:
//     ~PtyProcessImpl()
//     {
//         QTC_CHECK(m_setup.m_ptyData);
//         m_setup.m_ptyData->setResizeHandler({});
//     }

//     qint64 write(const QByteArray &data) final
//     {
//         if (m_ptyProcess)
//             return m_ptyProcess->write(data);
//         return -1;
//     }

//     void sendControlSignal(ControlSignal controlSignal) final
//     {
//         if (!m_ptyProcess)
//             return;

//         switch (controlSignal) {
//         case ControlSignal::Terminate: m_ptyProcess.reset(); break;
//         case ControlSignal::Kill: m_ptyProcess->kill(); break;
//         default: QTC_CHECK(false);
//         }
//     }

//     void doDefaultStart(const QString &program, const QStringList &arguments) final
//     {
//         QString executable = program;
//         FilePath path = FilePath::fromUserInput(executable);
//         if (!path.isAbsolutePath()) {
//             path = path.searchInPath();
//             if (path.isEmpty()) {
//                 const ProcessResultData result
//                     = {0,
//                        QProcess::CrashExit,
//                        QProcess::FailedToStart,
//                        Tr::tr("The program \"%1\" could not be found.").arg(program)};
//                 emit done(result);
//                 return;
//             }

//             executable = path.nativePath();
//         }

//         QTC_CHECK(m_setup.m_ptyData);
//         m_setup.m_ptyData->setResizeHandler([this](const QSize &size) {
//             if (m_ptyProcess)
//                 m_ptyProcess->resize(size.width(), size.height());
//         });
//         m_ptyProcess.reset(PtyQt::createPtyProcess(IPtyProcess::AutoPty));
//         if (!m_ptyProcess) {
//             const ProcessResultData result = {-1,
//                                               QProcess::CrashExit,
//                                               QProcess::FailedToStart,
//                                               "Failed to create pty process"};
//             emit done(result);
//             return;
//         }

//         QProcessEnvironment penv = m_setup.m_environment.toProcessEnvironment();
//         if (penv.isEmpty())
//             penv = Environment::systemEnvironment().toProcessEnvironment();
//         const QStringList senv = penv.toStringList();

//         FilePath workingDir = m_setup.m_workingDirectory;
//         if (!workingDir.isDir())
//             workingDir = workingDir.parentDir();
//         if (!QTC_GUARD(workingDir.exists()))
//             workingDir = workingDir.withNewPath({});

//         connect(m_ptyProcess->notifier(), &QIODevice::readyRead, this, [this] {
//             if (m_setup.m_ptyData->ptyInputFlagsChangedHandler()
//                 && m_inputFlags != m_ptyProcess->inputFlags()) {
//                 m_inputFlags = m_ptyProcess->inputFlags();
//                 m_setup.m_ptyData->ptyInputFlagsChangedHandler()(
//                     static_cast<Pty::PtyInputFlag>(m_inputFlags.toInt()));
//             }

//             const QByteArray data = m_ptyProcess->readAll();
//             if (!data.isEmpty())
//                 emit readyRead(data, {});
//         });

//         connect(m_ptyProcess->notifier(), &QIODevice::aboutToClose, this, [this] {
//             if (m_ptyProcess) {
//                 const ProcessResultData result = {m_ptyProcess->exitCode(),
//                                                   QProcess::NormalExit,
//                                                   QProcess::UnknownError,
//                                                   {}};

//                 const QByteArray restOfOutput = m_ptyProcess->readAll();
//                 if (!restOfOutput.isEmpty()) {
//                     emit readyRead(restOfOutput, {});
//                     m_ptyProcess->notifier()->disconnect();
//                 }

//                 emit done(result);
//                 return;
//             }

//             const ProcessResultData result = {0, QProcess::NormalExit, QProcess::UnknownError, {}};
//             emit done(result);
//         });

//         bool startResult = m_ptyProcess->startProcess(executable,
//                                                       HostOsInfo::isWindowsHost()
//                                                           ? QStringList{m_setup.m_nativeArguments}
//                                                                 << arguments
//                                                           : arguments,
//                                                       workingDir.nativePath(),
//                                                       senv,
//                                                       m_setup.m_ptyData->size().width(),
//                                                       m_setup.m_ptyData->size().height());

//         if (!startResult) {
//             const ProcessResultData result = {-1,
//                                               QProcess::CrashExit,
//                                               QProcess::FailedToStart,
//                                               "Failed to start pty process: "
//                                                   + m_ptyProcess->lastError()};
//             emit done(result);
//             return;
//         }

//         if (!m_ptyProcess->lastError().isEmpty()) {
//             const ProcessResultData result = {-1,
//                                               QProcess::CrashExit,
//                                               QProcess::FailedToStart,
//                                               m_ptyProcess->lastError()};
//             emit done(result);
//             return;
//         }

//         emit started(m_ptyProcess->pid());
//     }

// private:
//     std::unique_ptr<IPtyProcess> m_ptyProcess;
//     IPtyProcess::PtyInputFlags m_inputFlags;
// };

class QProcessImpl final : public DefaultImpl
{
public:
    QProcessImpl()
        : m_process(new ProcessHelper(this))
    // , m_blockingImpl(new QProcessBlockingImpl(m_process))
    {
        connect(m_process, &QProcess::started, this, &QProcessImpl::handleStarted);
        connect(m_process, &QProcess::finished, this, &QProcessImpl::handleFinished);
        connect(m_process, &QProcess::errorOccurred, this, &QProcessImpl::handleError);
        connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
            emit readyRead(m_process->readAllStandardOutput(), {});
        });
        connect(m_process, &QProcess::readyReadStandardError, this, [this] {
            emit readyRead({}, m_process->readAllStandardError());
        });
    }
    ~QProcessImpl() final { ProcessReaper::reap(m_process, m_setup.m_reaperTimeout); }

private:
    qint64 write(const QByteArray &data) final { return m_process->write(data); }
    void sendControlSignal(ControlSignal controlSignal) final
    {
        switch (controlSignal) {
        case ControlSignal::Terminate: ProcessHelper::terminateProcess(m_process); break;
        case ControlSignal::Kill: m_process->kill(); break;
        case ControlSignal::Interrupt: ProcessHelper::interruptPid(m_process->processId()); break;
        case ControlSignal::KickOff: QTC_CHECK(false); break;
        case ControlSignal::CloseWriteChannel: m_process->closeWriteChannel(); break;
        }
    }

    void doDefaultStart(const QString &program, const QStringList &arguments) final
    {
        QTC_ASSERT(QThread::currentThread()->eventDispatcher(),
                   qWarning("Process::start(): Starting a process in a non QThread thread "
                            "may cause infinite hang when destroying the running process."));
        ProcessStartHandler *handler = m_process->processStartHandler();
        handler->setProcessMode(m_setup.m_processMode);
        handler->setWriteData(m_setup.m_writeData);
        handler->setNativeArguments(m_setup.m_nativeArguments);
        handler->setWindowsSpecificStartupFlags(m_setup.m_belowNormalPriority,
                                                m_setup.m_createConsoleOnWindows,
                                                m_setup.m_forceDefaultErrorMode);

        const QProcessEnvironment penv = m_setup.m_environment.toProcessEnvironment();
        if (!penv.isEmpty())
            m_process->setProcessEnvironment(penv);
        m_process->setWorkingDirectory(m_setup.m_workingDirectory.path());
        m_process->setStandardInputFile(m_setup.m_standardInputFile);
        m_process->setProcessChannelMode(m_setup.m_processChannelMode);
        if (m_setup.m_lowPriority)
            m_process->setLowPriority();
        if (m_setup.m_unixTerminalDisabled)
            m_process->setUnixTerminalDisabled();
        m_process->setUseCtrlCStub(m_setup.m_useCtrlCStub);
        m_process->setAllowCoreDumps(m_setup.m_allowCoreDumps);
        m_process->start(program, arguments, handler->openMode());
        handler->handleProcessStart();
    }

    void handleStarted()
    {
        m_process->processStartHandler()->handleProcessStarted();
        emit started(m_process->processId());
    }

    void handleError(QProcess::ProcessError error)
    {
        if (error != QProcess::FailedToStart)
            return;
        const ProcessResultData result = {m_process->exitCode(),
                                          m_process->exitStatus(),
                                          error,
                                          m_process->errorString()};
        emit done(result);
    }

    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        const ProcessResultData result = {exitCode,
                                          exitStatus,
                                          m_process->error(),
                                          m_process->errorString()};
        emit done(result);
    }

    ProcessHelper *m_process = nullptr;
    // QProcessBlockingImpl *m_blockingImpl = nullptr;
};

class ProcessInterfaceSignal
{
public:
    ProcessSignalType signalType() const { return m_signalType; }
    virtual ~ProcessInterfaceSignal() = default;

protected:
    ProcessInterfaceSignal(ProcessSignalType signalType)
        : m_signalType(signalType)
    {}

private:
    const ProcessSignalType m_signalType;
};

class StartedSignal : public ProcessInterfaceSignal
{
public:
    StartedSignal(qint64 processId, qint64 applicationMainThreadId)
        : ProcessInterfaceSignal(ProcessSignalType::Started)
        , m_processId(processId)
        , m_applicationMainThreadId(applicationMainThreadId)
    {}
    qint64 processId() const { return m_processId; }
    qint64 applicationMainThreadId() const { return m_applicationMainThreadId; }

private:
    const qint64 m_processId;
    const qint64 m_applicationMainThreadId;
};

class ReadyReadSignal : public ProcessInterfaceSignal
{
public:
    ReadyReadSignal(const QByteArray &stdOut, const QByteArray &stdErr)
        : ProcessInterfaceSignal(ProcessSignalType::ReadyRead)
        , m_stdOut(stdOut)
        , m_stdErr(stdErr)
    {}
    QByteArray stdOut() const { return m_stdOut; }
    QByteArray stdErr() const { return m_stdErr; }

private:
    const QByteArray m_stdOut;
    const QByteArray m_stdErr;
};

class DoneSignal : public ProcessInterfaceSignal
{
public:
    DoneSignal(const ProcessResultData &resultData)
        : ProcessInterfaceSignal(ProcessSignalType::Done)
        , m_resultData(resultData)
    {}
    ProcessResultData resultData() const { return m_resultData; }

private:
    const ProcessResultData m_resultData;
};

class ProcessBlockingInterface;

class ProcessInterfaceHandler : public QObject
{
public:
    ProcessInterfaceHandler(ProcessBlockingInterface *caller, ProcessInterface *process);

    // Called from caller's thread exclusively.
    bool waitForSignal(ProcessSignalType newSignal, QDeadlineTimer timeout);
    void moveToCallerThread();

private:
    // Called from caller's thread exclusively.
    bool doWaitForSignal(QDeadlineTimer deadline);

    // Called from caller's thread when not waiting for signal,
    // otherwise called from temporary thread.
    void handleStarted(qint64 processId, qint64 applicationMainThreadId);
    void handleReadyRead(const QByteArray &outputData, const QByteArray &errorData);
    void handleDone(const ProcessResultData &data);
    void appendSignal(ProcessInterfaceSignal *newSignal);

    ProcessBlockingInterface *m_caller = nullptr;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
};

class ProcessBlockingInterface : public QObject
{
public:
    ProcessBlockingInterface(ProcessPrivate *parent);

    // Wait for:
    // - Started is being called only in Starting state.
    // - ReadyRead is being called in Starting or Running state.
    // - Done is being called in Starting or Running state.
    bool waitForSignal(ProcessSignalType signalType, QDeadlineTimer timeout);

private:
    void flush() { flushSignals(takeAllSignals()); }
    bool flushFor(ProcessSignalType signalType)
    {
        return flushSignals(takeSignalsFor(signalType), &signalType);
    }

    bool shouldFlush() const
    {
        QMutexLocker locker(&m_mutex);
        return !m_signals.isEmpty();
    }
    // Called from ProcessInterfaceHandler thread exclusively.
    void appendSignal(ProcessInterfaceSignal *launcherSignal);

    QList<ProcessInterfaceSignal *> takeAllSignals();
    QList<ProcessInterfaceSignal *> takeSignalsFor(ProcessSignalType signalType);
    bool flushSignals(const QList<ProcessInterfaceSignal *> &signalList,
                      ProcessSignalType *signalType = nullptr);

    void handleStartedSignal(const StartedSignal *launcherSignal);
    void handleReadyReadSignal(const ReadyReadSignal *launcherSignal);
    void handleDoneSignal(const DoneSignal *launcherSignal);

    friend class ProcessInterfaceHandler;
    ProcessPrivate *m_caller = nullptr;
    std::unique_ptr<ProcessInterfaceHandler> m_processHandler;
    mutable QMutex m_mutex;
    QList<ProcessInterfaceSignal *> m_signals;
    Guard m_guard;
};

class ProcessPrivate : public QObject
{
public:
    explicit ProcessPrivate(Process *parent)
        : QObject(parent)
        , q(parent)
        , m_killTimer(this)
    {
        m_killTimer.setSingleShot(true);
        connect(&m_killTimer, &QTimer::timeout, this, [this] {
            m_killTimer.stop();
            emit q->stoppingForcefully();
            sendControlSignal(ControlSignal::Kill);
        });
        setupDebugLog();
    }

    void setupDebugLog();
    void storeEventLoopDebugInfo(const QVariant &value);

    ProcessInterface *createProcessInterface()
    {
        // if (m_setup.m_ptyData)
        //     return new PtyProcessImpl;
        // if (m_setup.m_terminalMode != TerminalMode::Off)
        //     return Terminal::Hooks::instance().createTerminalProcessInterface();
        return new QProcessImpl;
    }

    void setProcessInterface(ProcessInterface *process)
    {
        if (m_process)
            m_process->disconnect();
        m_process.reset(process);
        m_process->setParent(this);
        connect(m_process.get(), &ProcessInterface::started, this, &ProcessPrivate::handleStarted);
        connect(m_process.get(),
                &ProcessInterface::readyRead,
                this,
                &ProcessPrivate::handleReadyRead);
        connect(m_process.get(), &ProcessInterface::done, this, &ProcessPrivate::handleDone);

        m_blockingInterface.reset(new ProcessBlockingInterface(this));
        m_blockingInterface->setParent(this);
    }

    Process *q;
    std::unique_ptr<ProcessBlockingInterface> m_blockingInterface;
    std::unique_ptr<ProcessInterface> m_process;
    ProcessSetupData m_setup;

    void handleStarted(qint64 processId, qint64 applicationMainThreadId);
    void handleReadyRead(const QByteArray &outputData, const QByteArray &errorData);
    void handleDone(const ProcessResultData &data);
    void clearForRun();

    void emitGuardedSignal(void (Process::*signalName)())
    {
        GuardLocker locker(m_guard);
        emit(q->*signalName)();
    }

    bool waitForSignal(ProcessSignalType signalType, QDeadlineTimer timeout);
    Qt::ConnectionType connectionType() const;
    void sendControlSignal(ControlSignal controlSignal);

    QTimer m_killTimer;
    QProcess::ProcessState m_state = QProcess::NotRunning;
    qint64 m_processId = 0;
    qint64 m_applicationMainThreadId = 0;
    ProcessResultData m_resultData;

    TextCodec m_stdOutCodec;
    TextCodec m_stdErrCodec;

    ProcessResult m_result = ProcessResult::StartFailed;
    ChannelBuffer m_stdOut;
    ChannelBuffer m_stdErr;

    time_point<system_clock, nanoseconds> m_startTimestamp = {};
    time_point<system_clock, nanoseconds> m_doneTimestamp = {};
    bool m_timeOutMessageBoxEnabled = false;

    Guard m_guard;
};

ProcessInterfaceHandler::ProcessInterfaceHandler(ProcessBlockingInterface *caller,
                                                 ProcessInterface *process)
    : m_caller(caller)
{
    process->disconnect();
    connect(process, &ProcessInterface::started, this, &ProcessInterfaceHandler::handleStarted);
    connect(process, &ProcessInterface::readyRead, this, &ProcessInterfaceHandler::handleReadyRead);
    connect(process, &ProcessInterface::done, this, &ProcessInterfaceHandler::handleDone);
}

// Called from caller's thread exclusively.
bool ProcessInterfaceHandler::waitForSignal(ProcessSignalType newSignal, QDeadlineTimer timeout)
{
    while (true) {
        if (timeout.hasExpired())
            break;
        if (!doWaitForSignal(timeout))
            break;
        // Matching (or Done) signal was flushed
        if (m_caller->flushFor(newSignal))
            return true;
        // Otherwise continue awaiting (e.g. when ReadyRead came while waitForFinished())
    }
    return false;
}

// Called from caller's thread exclusively.
void ProcessInterfaceHandler::moveToCallerThread()
{
    QMetaObject::invokeMethod(
        this, [this] { moveToThread(m_caller->thread()); }, Qt::BlockingQueuedConnection);
}

// Called from caller's thread exclusively.
bool ProcessInterfaceHandler::doWaitForSignal(QDeadlineTimer deadline)
{
    QMutexLocker locker(&m_mutex);

    // Flush, if we have any stored signals.
    // This must be called when holding laucher's mutex locked prior to the call to wait,
    // so that it's done atomically.
    if (m_caller->shouldFlush())
        return true;

    return m_waitCondition.wait(&m_mutex, deadline);
}

// Called from ProcessInterfaceHandler thread exclusively
void ProcessInterfaceHandler::handleStarted(qint64 processId, qint64 applicationMainThreadId)
{
    appendSignal(new StartedSignal(processId, applicationMainThreadId));
}

// Called from ProcessInterfaceHandler thread exclusively
void ProcessInterfaceHandler::handleReadyRead(const QByteArray &outputData,
                                              const QByteArray &errorData)
{
    appendSignal(new ReadyReadSignal(outputData, errorData));
}

// Called from ProcessInterfaceHandler thread exclusively
void ProcessInterfaceHandler::handleDone(const ProcessResultData &data)
{
    appendSignal(new DoneSignal(data));
}

void ProcessInterfaceHandler::appendSignal(ProcessInterfaceSignal *newSignal)
{
    {
        QMutexLocker locker(&m_mutex);
        m_caller->appendSignal(newSignal);
    }
    m_waitCondition.wakeOne();
    // call in callers thread
    QMetaObject::invokeMethod(m_caller, &ProcessBlockingInterface::flush);
}

ProcessBlockingInterface::ProcessBlockingInterface(ProcessPrivate *parent)
    : m_caller(parent)
    , m_processHandler(new ProcessInterfaceHandler(this, parent->m_process.get()))
{
    // In order to move the process interface into another thread together with handle
    parent->m_process.get()->setParent(m_processHandler.get());
    m_processHandler->setParent(this);
    // So the hierarchy looks like:
    // ProcessPrivate
    //  |
    //  +- ProcessBlockingInterface
    //      |
    //      +- ProcessInterfaceHandler
    //          |
    //          +- ProcessInterface
}

bool ProcessBlockingInterface::waitForSignal(ProcessSignalType newSignal, QDeadlineTimer timeout)
{
    QTC_ASSERT(!m_guard.isLocked(),
               qWarning("Process::waitForSignal() called recursively. "
                        "The call is being ignored.");
               return false);
    GuardLocker locker(m_guard);

    m_processHandler->setParent(nullptr);

    QThread thread;
    thread.start();
    // Note: the thread may have started before and it's appending new signals before
    // waitForSignal() is called. However, in this case they won't be flushed since
    // the caller here is blocked, so all signals should be buffered and we are going
    // to flush them from inside waitForSignal().
    m_processHandler->moveToThread(&thread);
    const bool result = m_processHandler->waitForSignal(newSignal, timeout);
    m_processHandler->moveToCallerThread();
    m_processHandler->setParent(this);
    thread.quit();
    thread.wait();
    return result;
}

// Called from caller's thread exclusively
QList<ProcessInterfaceSignal *> ProcessBlockingInterface::takeAllSignals()
{
    QMutexLocker locker(&m_mutex);
    return std::exchange(m_signals, {});
}

// Called from caller's thread exclusively
QList<ProcessInterfaceSignal *> ProcessBlockingInterface::takeSignalsFor(ProcessSignalType signalType)
{
    // If we are flushing for ReadyRead or Done - flush all.
    if (signalType != ProcessSignalType::Started)
        return takeAllSignals();

    QMutexLocker locker(&m_mutex);
    const QList<ProcessSignalType> storedSignals
        = transform(std::as_const(m_signals),
                    [](const ProcessInterfaceSignal *aSignal) { return aSignal->signalType(); });

    // If we are flushing for Started:
    // - if Started was buffered - flush Started only (even when Done was buffered)
    // - otherwise if Done signal was buffered - flush all.
    if (!storedSignals.contains(ProcessSignalType::Started)
        && storedSignals.contains(ProcessSignalType::Done)) {
        return std::exchange(m_signals, {}); // avoid takeAllSignals() because of mutex locked
    }

    QList<ProcessInterfaceSignal *> oldSignals;
    const auto matchingIndex = storedSignals.lastIndexOf(signalType);
    if (matchingIndex >= 0) {
        oldSignals = m_signals.mid(0, matchingIndex + 1);
        m_signals = m_signals.mid(matchingIndex + 1);
    }
    return oldSignals;
}

// Called from caller's thread exclusively
bool ProcessBlockingInterface::flushSignals(const QList<ProcessInterfaceSignal *> &signalList,
                                            ProcessSignalType *signalType)
{
    bool signalMatched = false;
    for (const ProcessInterfaceSignal *storedSignal : std::as_const(signalList)) {
        const ProcessSignalType storedSignalType = storedSignal->signalType();
        if (signalType && storedSignalType == *signalType)
            signalMatched = true;
        switch (storedSignalType) {
        case ProcessSignalType::Started:
            handleStartedSignal(static_cast<const StartedSignal *>(storedSignal));
            break;
        case ProcessSignalType::ReadyRead:
            handleReadyReadSignal(static_cast<const ReadyReadSignal *>(storedSignal));
            break;
        case ProcessSignalType::Done:
            if (signalType)
                signalMatched = true;
            handleDoneSignal(static_cast<const DoneSignal *>(storedSignal));
            break;
        }
        delete storedSignal;
    }
    return signalMatched;
}

void ProcessBlockingInterface::handleStartedSignal(const StartedSignal *aSignal)
{
    m_caller->handleStarted(aSignal->processId(), aSignal->applicationMainThreadId());
}

void ProcessBlockingInterface::handleReadyReadSignal(const ReadyReadSignal *aSignal)
{
    m_caller->handleReadyRead(aSignal->stdOut(), aSignal->stdErr());
}

void ProcessBlockingInterface::handleDoneSignal(const DoneSignal *aSignal)
{
    m_caller->handleDone(aSignal->resultData());
}

// Called from ProcessInterfaceHandler thread exclusively.
void ProcessBlockingInterface::appendSignal(ProcessInterfaceSignal *newSignal)
{
    QMutexLocker locker(&m_mutex);
    m_signals.append(newSignal);
}

bool ProcessPrivate::waitForSignal(ProcessSignalType newSignal, QDeadlineTimer timeout)
{
    const QDeadlineTimer currentKillTimeout(m_killTimer.remainingTime());
    const bool needsSplit = m_killTimer.isActive() && timeout > currentKillTimeout;
    const QDeadlineTimer mainTimeout = needsSplit ? currentKillTimeout : timeout;

    bool result = m_blockingInterface->waitForSignal(newSignal,
                                                     duration_cast<milliseconds>(
                                                         mainTimeout.remainingTimeAsDuration()));
    if (!result && needsSplit) {
        m_killTimer.stop();
        sendControlSignal(ControlSignal::Kill);
        result = m_blockingInterface->waitForSignal(newSignal, timeout);
    }
    return result;
}

Qt::ConnectionType ProcessPrivate::connectionType() const
{
    return (m_process->thread() == thread()) ? Qt::DirectConnection : Qt::BlockingQueuedConnection;
}

void ProcessPrivate::sendControlSignal(ControlSignal controlSignal)
{
    QTC_ASSERT(QThread::currentThread() == thread(), return);
    if (!m_process || (m_state == QProcess::NotRunning))
        return;

    if (controlSignal == ControlSignal::Terminate || controlSignal == ControlSignal::Kill) {
        m_doneTimestamp = system_clock::now();
        m_result = ProcessResult::Canceled;
    }

    QMetaObject::invokeMethod(
        m_process.get(),
        [this, controlSignal] { m_process->sendControlSignal(controlSignal); },
        connectionType());
}

void ProcessPrivate::clearForRun()
{
    if (!m_stdOutCodec.isValid())
        m_stdOutCodec = m_setup.m_commandLine.executable().processStdOutCodec();
    m_stdOut.clearForRun();
    m_stdOut.codec = m_stdOutCodec;

    if (!m_stdErrCodec.isValid())
        m_stdErrCodec = m_setup.m_commandLine.executable().processStdErrCodec();
    m_stdErr.clearForRun();
    m_stdErr.codec = m_stdErrCodec;

    m_result = ProcessResult::StartFailed;
    m_startTimestamp = {};
    m_doneTimestamp = {};

    m_killTimer.stop();
    m_state = QProcess::NotRunning;
    m_processId = 0;
    m_applicationMainThreadId = 0;
    m_resultData = {};
}

} // namespace Internal

/*!
    \class Utils::Process
    \inmodule QtCreator

    \brief The Process class provides functionality for with processes.

    \sa Utils::ProcessArgs
*/

Process::Process(QObject *parent)
    : QObject(parent)
    , d(new ProcessPrivate(this))
{
    qRegisterMetaType<ProcessResultData>("ProcessResultData");
    static int qProcessExitStatusMeta = qRegisterMetaType<QProcess::ExitStatus>();
    static int qProcessProcessErrorMeta = qRegisterMetaType<QProcess::ProcessError>();
    Q_UNUSED(qProcessExitStatusMeta)
    Q_UNUSED(qProcessProcessErrorMeta)
}

Process::~Process()
{
    QTC_ASSERT(!d->m_guard.isLocked(),
               qWarning("Deleting Process instance directly from "
                        "one of its signal handlers will lead to crash!"));
    if (d->m_process)
        d->m_process->disconnect();
    delete d;
}

void Process::setPtyData(const std::optional<Pty::Data> &data)
{
    d->m_setup.m_ptyData = data;
}

std::optional<Pty::Data> Process::ptyData() const
{
    return d->m_setup.m_ptyData;
}

ProcessMode Process::processMode() const
{
    return d->m_setup.m_processMode;
}

void Process::setTerminalMode(TerminalMode mode)
{
    d->m_setup.m_terminalMode = mode;
}

TerminalMode Process::terminalMode() const
{
    return d->m_setup.m_terminalMode;
}

void Process::setProcessMode(ProcessMode processMode)
{
    d->m_setup.m_processMode = processMode;
}

void Process::setEnvironment(const Environment &env)
{
    d->m_setup.m_environment = env;
}

const Environment &Process::environment() const
{
    return d->m_setup.m_environment;
}

void Process::setControlEnvironment(const Environment &environment)
{
    d->m_setup.m_controlEnvironment = environment;
}

const Environment &Process::controlEnvironment() const
{
    return d->m_setup.m_controlEnvironment;
}

void Process::setRunData(const ProcessRunData &data)
{
    if (!data.workingDirectory.isLocal() && !data.command.executable().isLocal()) {
        QTC_CHECK(data.workingDirectory.isSameDevice(data.command.executable()));
    }
    d->m_setup.m_commandLine = data.command;
    d->m_setup.m_workingDirectory = data.workingDirectory;
    d->m_setup.m_environment = data.environment;
}

ProcessRunData Process::runData() const
{
    return {d->m_setup.m_commandLine, d->m_setup.m_workingDirectory, d->m_setup.m_environment};
}

void Process::setCommand(const CommandLine &cmdLine)
{
    if (!d->m_setup.m_workingDirectory.isLocal() && !cmdLine.executable().isLocal()) {
        QTC_CHECK(d->m_setup.m_workingDirectory.isSameDevice(cmdLine.executable()));
    }
    d->m_setup.m_commandLine = cmdLine;
}

const CommandLine &Process::commandLine() const
{
    return d->m_setup.m_commandLine;
}

FilePath Process::workingDirectory() const
{
    return d->m_setup.m_workingDirectory;
}

void Process::setWorkingDirectory(const FilePath &dir)
{
    if (!dir.isLocal() && !d->m_setup.m_commandLine.executable().isLocal()) {
        QTC_CHECK(dir.isSameDevice(d->m_setup.m_commandLine.executable()));
    }
    d->m_setup.m_workingDirectory = dir;
}

void Process::setUseCtrlCStub(bool enabled)
{
    d->m_setup.m_useCtrlCStub = enabled;
}

void Process::setAllowCoreDumps(bool enabled)
{
    d->m_setup.m_allowCoreDumps = enabled;
}

void Process::start()
{
    QTC_ASSERT(state() == QProcess::NotRunning, return);
    QTC_ASSERT(!(d->m_process && d->m_guard.isLocked()),
               qWarning("Restarting the Process directly from one of its signal handlers will "
                        "lead to crash! Consider calling close() prior to direct restart."));
    d->clearForRun();

    if (d->m_setup.m_commandLine.executable().isEmpty()
        && d->m_setup.m_commandLine.executable().scheme().isEmpty()
        && d->m_setup.m_commandLine.executable().host().isEmpty()) {
        d->m_result = ProcessResult::StartFailed;
        d->m_resultData.m_exitCode = 255;
        d->m_resultData.m_exitStatus = QProcess::CrashExit;
        d->m_resultData.m_errorString = Tr::tr("No executable specified.");
        d->m_resultData.m_error = QProcess::FailedToStart;
        d->emitGuardedSignal(&Process::done);
        return;
    }

    ProcessInterface *processImpl = nullptr;
    if (d->m_setup.m_commandLine.executable().isLocal()) {
        processImpl = d->createProcessInterface();
    } else {
        QTC_ASSERT(s_deviceHooks.processImplHook, return);
        processImpl = s_deviceHooks.processImplHook(commandLine().executable());
    }

    if (!processImpl) {
        // This happens if a device does not implement the createProcessInterface() function.
        d->m_result = ProcessResult::StartFailed;
        d->m_resultData.m_exitCode = 255;
        d->m_resultData.m_exitStatus = QProcess::CrashExit;
        d->m_resultData.m_errorString = Tr::tr("Failed to create process interface for \"%1\".")
                                            .arg(d->m_setup.m_commandLine.toUserOutput());
        d->m_resultData.m_error = QProcess::FailedToStart;
        d->emitGuardedSignal(&Process::done);
        return;
    }

    d->setProcessInterface(processImpl);
    d->m_state = QProcess::Starting;
    d->m_process->m_setup = d->m_setup;
    d->m_startTimestamp = system_clock::now();
    d->emitGuardedSignal(&Process::starting);
    d->m_process->start();
}

void Process::terminate()
{
    d->sendControlSignal(ControlSignal::Terminate);
}

void Process::kill()
{
    d->sendControlSignal(ControlSignal::Kill);
}

void Process::interrupt()
{
    d->sendControlSignal(ControlSignal::Interrupt);
}

void Process::kickoffProcess()
{
    d->sendControlSignal(ControlSignal::KickOff);
}

void Process::closeWriteChannel()
{
    d->sendControlSignal(ControlSignal::CloseWriteChannel);
}

bool Process::startDetached(const CommandLine &cmd,
                            const FilePath &workingDirectory,
                            DetachedChannelMode channelMode,
                            qint64 *pid)
{
    QProcess process;
    process.setProgram(cmd.executable().toUserOutput());
    process.setArguments(cmd.splitArguments());
    process.setWorkingDirectory(workingDirectory.toUserOutput());
    if (channelMode == DetachedChannelMode::Discard) {
        process.setStandardOutputFile(QProcess::nullDevice());
        process.setStandardErrorFile(QProcess::nullDevice());
    }
    return process.startDetached(pid);
}

void Process::setLowPriority()
{
    d->m_setup.m_lowPriority = true;
}

void Process::setDisableUnixTerminal()
{
    d->m_setup.m_unixTerminalDisabled = true;
}

void Process::setAbortOnMetaChars(bool abort)
{
    d->m_setup.m_abortOnMetaChars = abort;
}

void Process::setRunAsRoot(bool on)
{
    d->m_setup.m_runAsRoot = on;
}

bool Process::isRunAsRoot() const
{
    return d->m_setup.m_runAsRoot;
}

void Process::setStandardInputFile(const QString &inputFile)
{
    d->m_setup.m_standardInputFile = inputFile;
}

QString Process::toStandaloneCommandLine() const
{
    QStringList parts;
    parts.append("/usr/bin/env");
    if (!d->m_setup.m_workingDirectory.isEmpty()) {
        parts.append("-C");
        parts.append(d->m_setup.m_workingDirectory.path());
    }
    parts.append("-i");
    if (d->m_setup.m_environment.hasChanges()) {
        const QStringList envVars = d->m_setup.m_environment.toStringList();
        std::transform(envVars.cbegin(),
                       envVars.cend(),
                       std::back_inserter(parts),
                       ProcessArgs::quoteArgUnix);
    }
    parts.append(d->m_setup.m_commandLine.executable().path());
    parts.append(d->m_setup.m_commandLine.splitArguments());
    return parts.join(" ");
}

void Process::setCreateConsoleOnWindows(bool create)
{
    d->m_setup.m_createConsoleOnWindows = create;
}

bool Process::createConsoleOnWindows() const
{
    return d->m_setup.m_createConsoleOnWindows;
}

void Process::setForceDefaultErrorModeOnWindows(bool force)
{
    d->m_setup.m_forceDefaultErrorMode = force;
}

bool Process::forceDefaultErrorModeOnWindows() const
{
    return d->m_setup.m_forceDefaultErrorMode;
}

void Process::setExtraData(const QString &key, const QVariant &value)
{
    d->m_setup.m_extraData.insert(key, value);
}

QVariant Process::extraData(const QString &key) const
{
    return d->m_setup.m_extraData.value(key);
}

void Process::setExtraData(const QVariantHash &extraData)
{
    d->m_setup.m_extraData = extraData;
}

QVariantHash Process::extraData() const
{
    return d->m_setup.m_extraData;
}

void Process::setReaperTimeout(milliseconds timeout)
{
    d->m_setup.m_reaperTimeout = timeout;
}

milliseconds Process::reaperTimeout() const
{
    return d->m_setup.m_reaperTimeout;
}

void Process::setRemoteProcessHooks(const DeviceProcessHooks &hooks)
{
    s_deviceHooks = hooks;
}

static bool askToKill(const CommandLine &command)
{
#ifdef QT_GUI_LIB
    if (!isGuiEnabled())
        return true;
    const QString title = Tr::tr("Process Not Responding");
    QString msg = command.isEmpty() ? Tr::tr("The process is not responding.")
                                    : Tr::tr("The process \"%1\" is not responding.")
                                          .arg(command.executable().toUserOutput());
    msg += ' ';
    msg += Tr::tr("Terminate the process?");
    // Restore the cursor that is set to wait while running.
    const bool hasOverrideCursor = QGuiApplication::overrideCursor() != nullptr;
    if (hasOverrideCursor)
        QGuiApplication::restoreOverrideCursor();
    QMessageBox::StandardButton answer = QMessageBox::question(nullptr,
                                                               title,
                                                               msg,
                                                               QMessageBox::Yes | QMessageBox::No);
    if (hasOverrideCursor)
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    return answer == QMessageBox::Yes;
#else
    Q_UNUSED(command)
    return true;
#endif
}

// Helper for running a process synchronously in the foreground with timeout
// detection (taking effect after no more output
// occurs on stderr/stdout as opposed to waitForFinished()). Returns false if a timeout
// occurs. Checking of the process' exit state/code still has to be done.

// TODO: Is it really needed?
bool Process::readDataFromProcess(QByteArray *stdOut, QByteArray *stdErr, int timeoutS)
{
    enum { syncDebug = 0 };
    if (syncDebug)
        qDebug() << ">readDataFromProcess" << timeoutS;
    if (state() != QProcess::Running) {
        qWarning("readDataFromProcess: Process in non-running state passed in.");
        return false;
    }

    // Keep the process running until it has no longer has data
    bool finished = false;
    bool hasData = false;
    do {
        finished = waitForFinished(timeoutS > 0 ? seconds(timeoutS) : seconds(-1))
                   || state() == QProcess::NotRunning;
        // First check 'stdout'
        const QByteArray newStdOut = readAllRawStandardOutput();
        if (!newStdOut.isEmpty()) {
            hasData = true;
            if (stdOut)
                stdOut->append(newStdOut);
        }
        // Check 'stderr' separately. This is a special handling
        // for 'git pull' and the like which prints its progress on stderr.
        const QByteArray newStdErr = readAllRawStandardError();
        if (!newStdErr.isEmpty()) {
            hasData = true;
            if (stdErr)
                stdErr->append(newStdErr);
        }
        // Prompt user, pretend we have data if says 'No'.
        const bool hang = !hasData && !finished;
        hasData = hang && !askToKill(d->m_setup.m_commandLine);
    } while (hasData && !finished);
    if (syncDebug)
        qDebug() << "<readDataFromProcess" << finished;
    return finished;
}

ProcessResult Process::result() const
{
    return d->m_result;
}

ProcessResultData Process::resultData() const
{
    return d->m_resultData;
}

int Process::exitCode() const
{
    return resultData().m_exitCode;
}

QProcess::ExitStatus Process::exitStatus() const
{
    return resultData().m_exitStatus;
}

QProcess::ProcessError Process::error() const
{
    return resultData().m_error;
}

QString Process::errorString() const
{
    return resultData().m_errorString;
}

qint64 Process::applicationMainThreadId() const
{
    return d->m_applicationMainThreadId;
}

QProcess::ProcessChannelMode Process::processChannelMode() const
{
    return d->m_setup.m_processChannelMode;
}

void Process::setProcessChannelMode(QProcess::ProcessChannelMode mode)
{
    d->m_setup.m_processChannelMode = mode;
}

QProcess::ProcessState Process::state() const
{
    return d->m_state;
}

bool Process::isRunning() const
{
    return state() == QProcess::Running;
}

qint64 Process::processId() const
{
    return d->m_processId;
}

bool Process::waitForStarted(QDeadlineTimer timeout)
{
    QTC_ASSERT(d->m_process, return false);
    if (d->m_state == QProcess::Running)
        return true;
    if (d->m_state == QProcess::NotRunning)
        return false;
    return s_waitForStarted.measureAndRun(&ProcessPrivate::waitForSignal,
                                          d,
                                          ProcessSignalType::Started,
                                          timeout);
}

bool Process::waitForReadyRead(QDeadlineTimer timeout)
{
    QTC_ASSERT(d->m_process, return false);
    if (d->m_state == QProcess::NotRunning)
        return false;
    return d->waitForSignal(ProcessSignalType::ReadyRead, timeout);
}

bool Process::waitForFinished(QDeadlineTimer timeout)
{
    QTC_ASSERT(d->m_process, return false);
    if (d->m_state == QProcess::NotRunning)
        return false;
    return d->waitForSignal(ProcessSignalType::Done, timeout);
}

QByteArray Process::readAllRawStandardOutput()
{
    return d->m_stdOut.readAllRawData();
}

QByteArray Process::readAllRawStandardError()
{
    return d->m_stdErr.readAllRawData();
}

qint64 Process::write(const QString &input)
{
    // Non-windows is assumed to be UTF-8
    if (commandLine().executable().osType() != OsTypeWindows)
        return writeRaw(input.toUtf8());

    if (HostOsInfo::hostOs() == OsTypeWindows)
        return writeRaw(input.toLocal8Bit());

    // "remote" Windows target on non-Windows host is unlikely,
    // but the true encoding is not accessible. Use UTF8 as best guess.
    QTC_CHECK(false);
    return writeRaw(input.toUtf8());
}

qint64 Process::writeRaw(const QByteArray &input)
{
    QTC_ASSERT(processMode() == ProcessMode::Writer, return -1);
    QTC_ASSERT(d->m_process, return -1);
    QTC_ASSERT(state() == QProcess::Running, return -1);
    QTC_ASSERT(QThread::currentThread() == thread(), return -1);
    qint64 result = -1;
    QMetaObject::invokeMethod(
        d->m_process.get(),
        [this, input] { return d->m_process->write(input); },
        d->connectionType(),
        &result);
    return result;
}

void Process::close()
{
    QTC_ASSERT(QThread::currentThread() == thread(), return);
    if (d->m_process) {
        // Note: the m_process may be inside ProcessInterfaceHandler's thread.
        QTC_ASSERT(d->m_process->thread() == thread(), return);
        d->m_process->disconnect();
        d->m_process.release()->deleteLater();
    }
    if (d->m_blockingInterface) {
        d->m_blockingInterface->disconnect();
        d->m_blockingInterface.release()->deleteLater();
    }
    d->clearForRun();
}

/*
   Calls terminate() directly and after a delay of reaperTimeout() it calls kill()
   if the process is still running.
*/
void Process::stop()
{
    if (state() == QProcess::NotRunning)
        return;

    d->sendControlSignal(ControlSignal::Terminate);
    d->m_killTimer.start(d->m_process->m_setup.m_reaperTimeout);
}

QString Process::readAllStandardOutput()
{
    return d->m_stdOut.readAllData();
}

QString Process::readAllStandardError()
{
    return d->m_stdErr.readAllData();
}

QString Process::exitMessage(const CommandLine &command,
                             ProcessResult result,
                             int exitCode,
                             milliseconds duration)
{
    const QString cmd = command.toUserOutput();
    switch (result) {
    case ProcessResult::FinishedWithSuccess:
        return Tr::tr("The command \"%1\" finished successfully.").arg(cmd);
    case ProcessResult::FinishedWithError:
        return Tr::tr("The command \"%1\" terminated with exit code %2.").arg(cmd).arg(exitCode);
    case ProcessResult::TerminatedAbnormally:
        return Tr::tr("The command \"%1\" terminated abnormally.").arg(cmd);
    case ProcessResult::StartFailed:
        return Tr::tr("The command \"%1\" could not be started.").arg(cmd) + ' '
               + Tr::tr("Either the invoked program is missing, or you may have insufficient "
                        "permissions to invoke the program.");
    case ProcessResult::Canceled:
        // TODO: We might want to format it nicely when bigger than 1 second, e.g. 1,324 s.
        //       Also when it's bigger than 1 minute, 1 hour, etc...
        return Tr::tr("The command \"%1\" was canceled after %2 ms.").arg(cmd).arg(duration.count());
    }
    return {};
}

QString Process::exitMessage(FailureMessageFormat format) const
{
    QString msg = exitMessage(commandLine(), result(), exitCode(), processDuration());
    if (format == FailureMessageFormat::Plain || result() == ProcessResult::FinishedWithSuccess)
        return msg;
    if (format == FailureMessageFormat::WithStdErr
        || format == FailureMessageFormat::WithAllOutput) {
        const QString stdErr = cleanedStdErr();
        if (!stdErr.isEmpty()) {
            msg.append('\n').append(Tr::tr("Standard error output was:")).append('\n').append(stdErr);
        }
    }
    if (format == FailureMessageFormat::WithStdOut
        || format == FailureMessageFormat::WithAllOutput) {
        const QString stdOut = cleanedStdOut();
        if (!stdOut.isEmpty())
            msg.append('\n').append(Tr::tr("Standard output was:")).append('\n').append(stdOut);
    }
    return msg;
}

milliseconds Process::processDuration() const
{
    if (d->m_startTimestamp == time_point<system_clock, nanoseconds>())
        return {};
    const auto end = (d->m_doneTimestamp == time_point<system_clock, nanoseconds>())
                         ? system_clock::now()
                         : d->m_doneTimestamp;
    return duration_cast<milliseconds>(end - d->m_startTimestamp);
}

QByteArray Process::allRawOutput() const
{
    QTC_CHECK(d->m_stdOut.keepRawData);
    QTC_CHECK(d->m_stdErr.keepRawData);
    if (!d->m_stdOut.rawData.isEmpty() && !d->m_stdErr.rawData.isEmpty()) {
        QByteArray result = d->m_stdOut.rawData;
        if (!result.endsWith('\n'))
            result += '\n';
        result += d->m_stdErr.rawData;
        return result;
    }
    return !d->m_stdOut.rawData.isEmpty() ? d->m_stdOut.rawData : d->m_stdErr.rawData;
}

QString Process::allOutput() const
{
    QTC_CHECK(d->m_stdOut.keepRawData);
    QTC_CHECK(d->m_stdErr.keepRawData);
    const QString out = cleanedStdOut();
    const QString err = cleanedStdErr();

    if (!out.isEmpty() && !err.isEmpty()) {
        QString result = out;
        if (!result.endsWith('\n'))
            result += '\n';
        result += err;
        return result;
    }
    return !out.isEmpty() ? out : err;
}

QByteArray Process::rawStdOut() const
{
    QTC_CHECK(d->m_stdOut.keepRawData);
    return d->m_stdOut.rawData;
}

QByteArray Process::rawStdErr() const
{
    QTC_CHECK(d->m_stdErr.keepRawData);
    return d->m_stdErr.rawData;
}

QString Process::stdOut() const
{
    QTC_CHECK(d->m_stdOut.keepRawData);
    QTC_ASSERT(d->m_stdOutCodec.isValid(), return {}); // Process was not started
    return d->m_stdOutCodec.toUnicode(d->m_stdOut.rawData);
}

QString Process::stdErr() const
{
    QTC_CHECK(d->m_stdErr.keepRawData);
    QTC_ASSERT(d->m_stdErrCodec.isValid(), return {}); // Process was not started
    return d->m_stdErrCodec.toUnicode(d->m_stdErr.rawData);
}

QString Process::cleanedStdOut() const
{
    return Utils::normalizeNewlines(stdOut());
}

QString Process::cleanedStdErr() const
{
    return Utils::normalizeNewlines(stdErr());
}

static QStringList splitLines(const QString &text)
{
    QStringList result = text.split('\n');
    for (QString &line : result) {
        if (line.endsWith('\r'))
            line.chop(1);
    }
    return result;
}

const QStringList Process::stdOutLines() const
{
    return splitLines(cleanedStdOut());
}

const QStringList Process::stdErrLines() const
{
    return splitLines(cleanedStdErr());
}

UTILS_EXPORT QDebug operator<<(QDebug str, const Process &r)
{
    QDebug nsp = str.nospace();
    nsp << "Process: result=" << int(r.d->m_result) << " ex=" << r.exitCode() << '\n'
        << r.d->m_stdOut.rawData.size() << " bytes stdout, stderr=" << r.d->m_stdErr.rawData
        << '\n';
    return str;
}

void ChannelBuffer::clearForRun()
{
    rawData.clear();
    codecState.reset();
    incompleteLineBuffer.clear();
}

/* Check for complete lines read from the device and return them, moving the
 * buffer position. */
void ChannelBuffer::append(const QByteArray &text)
{
    if (text.isEmpty())
        return;

    if (keepRawData)
        rawData += text;

    // Line-wise operation below:
    if (!outputCallback)
        return;

    // Convert and append the new input to the buffer of incomplete lines
    incompleteLineBuffer.append(codec.toUnicode(text.constData(), text.size(), &codecState));

    QStringView bufferView(incompleteLineBuffer);

    do {
        // Any completed lines in the bufferView?
        int pos = -1;
        if (emitSingleLines) {
            const int posn = bufferView.indexOf('\n');
            const int posr = bufferView.indexOf('\r');
            if (posn != -1) {
                if (posr != -1) {
                    if (posn == posr + 1)
                        pos = posn; // \r followed by \n -> line end, use the \n.
                    else
                        pos = qMin(posr, posn); // free floating \r and \n: Use the first one.
                } else {
                    pos = posn;
                }
            } else {
                pos = posr; // Make sure internal '\r' triggers a line output
            }
        } else {
            pos = qMax(bufferView.lastIndexOf('\n'), bufferView.lastIndexOf('\r'));
        }

        if (pos == -1)
            break;

        // Get completed lines and remove them from the incompleteLinesBuffer:
        const QString line = Utils::normalizeNewlines(bufferView.left(pos + 1));
        bufferView = bufferView.mid(pos + 1);

        QTC_ASSERT(outputCallback, return);
        outputCallback(line);

        if (!emitSingleLines)
            break;
    } while (true);
    incompleteLineBuffer = bufferView.toString();
}

void ChannelBuffer::handleRest()
{
    if (outputCallback && !incompleteLineBuffer.isEmpty()) {
        outputCallback(incompleteLineBuffer);
        incompleteLineBuffer.clear();
    }
}

void Process::setCodec(const TextCodec &codec)
{
    QTC_ASSERT(codec.isValid(), return);
    d->m_stdOutCodec = codec;
    d->m_stdErrCodec = codec;
}

void Process::setUtf8Codec()
{
    d->m_stdOutCodec = TextCodec::utf8();
    d->m_stdErrCodec = TextCodec::utf8();
}

void Process::setUtf8StdOutCodec()
{
    d->m_stdOutCodec = TextCodec::utf8();
}

void Process::setTimeOutMessageBoxEnabled(bool v)
{
    d->m_timeOutMessageBoxEnabled = v;
}

void Process::setWriteData(const QByteArray &writeData)
{
    d->m_setup.m_writeData = writeData;
}

void Process::runBlocking(seconds timeout, EventLoopMode eventLoopMode)
{
    QDateTime startTime;
    static const int blockingThresholdMs = qtcEnvironmentVariableIntValue("QTC_PROCESS_THRESHOLD");

    const auto handleStart = [this, eventLoopMode, &startTime] {
        // Attach a dynamic property with info about blocking type
        d->storeEventLoopDebugInfo(int(eventLoopMode));

        if (blockingThresholdMs > 0 && isMainThread())
            startTime = QDateTime::currentDateTime();
        start();

        // Remove the dynamic property so that it's not reused in subseqent start()
        d->storeEventLoopDebugInfo({});
    };

    const auto handleTimeout = [this] {
        if (state() == QProcess::NotRunning)
            return;
        stop();
        QTC_CHECK(waitForFinished(2s));
    };

    if (eventLoopMode == EventLoopMode::On) {
#ifdef QT_GUI_LIB
        if (isGuiEnabled())
            QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
        QEventLoop eventLoop(this);

        // Queue the call to start() so that it's executed after the nested event loop is started,
        // otherwise it fails on Windows with QProcessImpl. See QTCREATORBUG-30066.
        QMetaObject::invokeMethod(this, handleStart, Qt::QueuedConnection);

        std::function<void(void)> timeoutHandler = {};
        if (timeout > seconds::zero()) {
            timeoutHandler = [this, &eventLoop, &timeoutHandler, &handleTimeout, timeout] {
                if (!d->m_timeOutMessageBoxEnabled || askToKill(d->m_setup.m_commandLine)) {
                    handleTimeout();
                    return;
                }
                QTimer::singleShot(timeout, &eventLoop, timeoutHandler);
            };
            QTimer::singleShot(timeout, &eventLoop, timeoutHandler);
        }

        connect(this, &Process::done, &eventLoop, [&eventLoop] { eventLoop.quit(); });

        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
#ifdef QT_GUI_LIB
        if (isGuiEnabled())
            QGuiApplication::restoreOverrideCursor();
#endif
    } else {
        handleStart();
        if (state() != QProcess::NotRunning && !waitForFinished(timeout))
            handleTimeout();
    }
    if (blockingThresholdMs > 0) {
        const int timeDiff = startTime.msecsTo(QDateTime::currentDateTime());
        if (timeDiff > blockingThresholdMs && isMainThread()) {
            qWarning() << "Blocking process " << d->m_setup.m_commandLine << "took" << timeDiff
                       << "ms, longer than threshold" << blockingThresholdMs;
        }
    }
}

void Process::setStdOutCallback(const TextChannelCallback &callback)
{
    d->m_stdOut.outputCallback = callback;
    d->m_stdOut.emitSingleLines = false;
}

void Process::setStdOutLineCallback(const TextChannelCallback &callback)
{
    d->m_stdOut.outputCallback = callback;
    d->m_stdOut.emitSingleLines = true;
    d->m_stdOut.keepRawData = false;
}

void Process::setStdErrCallback(const TextChannelCallback &callback)
{
    d->m_stdErr.outputCallback = callback;
    d->m_stdErr.emitSingleLines = false;
}

void Process::setStdErrLineCallback(const TextChannelCallback &callback)
{
    d->m_stdErr.outputCallback = callback;
    d->m_stdErr.emitSingleLines = true;
    d->m_stdErr.keepRawData = false;
}

void Process::setTextChannelMode(Channel channel, TextChannelMode mode)
{
    const TextChannelCallback outputCb = [this](const QString &text) {
        GuardLocker locker(d->m_guard);
        emit textOnStandardOutput(text);
    };
    const TextChannelCallback errorCb = [this](const QString &text) {
        GuardLocker locker(d->m_guard);
        emit textOnStandardError(text);
    };
    const TextChannelCallback callback = (channel == Channel::Output) ? outputCb : errorCb;
    ChannelBuffer *buffer = channel == Channel::Output ? &d->m_stdOut : &d->m_stdErr;
    QTC_ASSERT(buffer->m_textChannelMode == TextChannelMode::Off,
               qWarning() << "Process::setTextChannelMode(): Changing text channel mode for"
                          << (channel == Channel::Output ? "Output" : "Error")
                          << "channel while it was previously set for this channel.");
    buffer->m_textChannelMode = mode;
    switch (mode) {
    case TextChannelMode::Off:
        buffer->outputCallback = {};
        buffer->emitSingleLines = true;
        buffer->keepRawData = true;
        break;
    case TextChannelMode::SingleLine:
        buffer->outputCallback = callback;
        buffer->emitSingleLines = true;
        buffer->keepRawData = false;
        break;
    case TextChannelMode::MultiLine:
        buffer->outputCallback = callback;
        buffer->emitSingleLines = false;
        buffer->keepRawData = true;
        break;
    }
}

TextChannelMode Process::textChannelMode(Channel channel) const
{
    ChannelBuffer *buffer = channel == Channel::Output ? &d->m_stdOut : &d->m_stdErr;
    return buffer->m_textChannelMode;
}

void ProcessPrivate::handleStarted(qint64 processId, qint64 applicationMainThreadId)
{
    QTC_CHECK(m_state == QProcess::Starting);
    m_state = QProcess::Running;

    m_processId = processId;
    m_applicationMainThreadId = applicationMainThreadId;
    emitGuardedSignal(&Process::started);
}

void ProcessPrivate::handleReadyRead(const QByteArray &outputData, const QByteArray &errorData)
{
    QTC_CHECK(m_state == QProcess::Running);

    // TODO: store a copy of m_processChannelMode on start()? Currently we assert that state
    // is NotRunning when setting the process channel mode.

    if (!outputData.isEmpty()) {
        if (m_process->m_setup.m_processChannelMode == QProcess::ForwardedOutputChannel
            || m_process->m_setup.m_processChannelMode == QProcess::ForwardedChannels) {
            std::cout << outputData.constData() << std::flush;
        } else {
            m_stdOut.append(outputData);
            emitGuardedSignal(&Process::readyReadStandardOutput);
        }
    }
    if (!errorData.isEmpty()) {
        if (m_process->m_setup.m_processChannelMode == QProcess::ForwardedErrorChannel
            || m_process->m_setup.m_processChannelMode == QProcess::ForwardedChannels) {
            std::cerr << errorData.constData() << std::flush;
        } else {
            m_stdErr.append(errorData);
            emitGuardedSignal(&Process::readyReadStandardError);
        }
    }
}

void ProcessPrivate::handleDone(const ProcessResultData &data)
{
    if (m_result != ProcessResult::Canceled)
        m_doneTimestamp = system_clock::now();
    m_killTimer.stop();
    m_resultData = data;

    switch (m_state) {
    case QProcess::NotRunning:
        QTC_ASSERT(false, return); // Can't happen
        break;
    case QProcess::Starting: QTC_CHECK(m_resultData.m_error == QProcess::FailedToStart); break;
    case QProcess::Running: QTC_CHECK(m_resultData.m_error != QProcess::FailedToStart); break;
    }
    m_state = QProcess::NotRunning;

    // This code (255) is being returned by QProcess when FailedToStart error occurred
    if (m_resultData.m_error == QProcess::FailedToStart)
        m_resultData.m_exitCode = 0xFF;

    // HACK: See QIODevice::errorString() implementation.
    if (m_resultData.m_error == QProcess::UnknownError)
        m_resultData.m_errorString.clear();

    if (m_result != ProcessResult::Canceled && m_resultData.m_error != QProcess::FailedToStart) {
        switch (m_resultData.m_exitStatus) {
        case QProcess::NormalExit:
            m_result = m_resultData.m_exitCode ? ProcessResult::FinishedWithError
                                               : ProcessResult::FinishedWithSuccess;
            break;
        case QProcess::CrashExit: m_result = ProcessResult::TerminatedAbnormally; break;
        }
    }

    m_stdOut.handleRest();
    m_stdErr.handleRest();

    emitGuardedSignal(&Process::done);
    m_processId = 0;
    m_applicationMainThreadId = 0;
}

static QString blockingMessage(const QVariant &variant)
{
    if (!variant.isValid())
        return "non blocking";
    if (variant.toInt() == int(EventLoopMode::On))
        return "blocking with event loop";
    return "blocking without event loop";
}

void ProcessPrivate::setupDebugLog()
{
    if (!processLog().isDebugEnabled())
        return;

    auto now = [] {
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    };

    connect(q, &Process::starting, this, [this, now] {
        const quint64 msNow = now();
        setProperty(QTC_PROCESS_STARTTIME, msNow);

        static std::atomic_int startCounter = 0;
        const int currentNumber = startCounter.fetch_add(1);
        qCDebug(processLog).nospace().noquote()
            << "Process " << currentNumber << " starting ("
            << qPrintable(blockingMessage(property(QTC_PROCESS_BLOCKING_TYPE)))
            << (isMainThread() ? ", main thread" : "")
            << "): " << m_setup.m_commandLine.toUserOutput();
        setProperty(QTC_PROCESS_NUMBER, currentNumber);
    });

    connect(q, &Process::done, this, [this, now] {
        if (!m_process.get())
            return;
        const QVariant n = property(QTC_PROCESS_NUMBER);
        if (!n.isValid())
            return;
        const quint64 msNow = now();
        const quint64 msStarted = property(QTC_PROCESS_STARTTIME).toULongLong();
        const quint64 msElapsed = msNow - msStarted;

        const int number = n.toInt();
        const QString stdOut = q->cleanedStdOut();
        const QString stdErr = q->cleanedStdErr();
        qCDebug(processLog).nospace()
            << "Process " << number << " finished: result=" << int(m_result)
            << ", ex=" << m_resultData.m_exitCode << ", " << stdOut.size()
            << " bytes stdout: " << stdOut.left(20) << ", " << stdErr.size()
            << " bytes stderr: " << stdErr.left(1000) << ", " << msElapsed << " ms elapsed";
        if (processStdoutLog().isDebugEnabled() && !stdOut.isEmpty())
            qCDebug(processStdoutLog).nospace() << "Process " << number << " sdout: " << stdOut;
        if (processStderrLog().isDebugEnabled() && !stdErr.isEmpty())
            qCDebug(processStderrLog).nospace() << "Process " << number << " stderr: " << stdErr;
    });
}

void ProcessPrivate::storeEventLoopDebugInfo(const QVariant &value)
{
    if (processLog().isDebugEnabled())
        setProperty(QTC_PROCESS_BLOCKING_TYPE, value);
}

ProcessTaskAdapter::ProcessTaskAdapter()
{
    connect(task(), &Process::done, this, [this] {
        emit done(Tasking::toDoneResult(task()->result() == ProcessResult::FinishedWithSuccess));
    });
}

void ProcessTaskAdapter::start()
{
    task()->start();
}

} // namespace Utils
