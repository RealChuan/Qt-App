// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include "commandline.h"
#include "environment.h"
#include "processenums.h"

#include <QDeadlineTimer>
#include <QProcess>
#include <QSize>

namespace Utils {

namespace Internal {
class ProcessPrivate;
}

namespace Pty {

enum PtyInputFlag {
    None = 0x0,
    InputModeHidden = 0x1,
};

using ResizeHandler = std::function<void(const QSize &)>;
using PtyInputFlagsChangeHandler = std::function<void(PtyInputFlag)>;

class UTILS_EXPORT SharedData
{
public:
    ResizeHandler m_handler;
    PtyInputFlagsChangeHandler m_inputFlagsChangedHandler;
};

class UTILS_EXPORT Data
{
public:
    Data()
        : m_data(new SharedData)
    {}

    void setResizeHandler(const ResizeHandler &handler) { m_data->m_handler = handler; }
    void setPtyInputFlagsChangedHandler(const PtyInputFlagsChangeHandler &handler)
    {
        m_data->m_inputFlagsChangedHandler = handler;
    }

    PtyInputFlagsChangeHandler ptyInputFlagsChangedHandler() const
    {
        return m_data->m_inputFlagsChangedHandler;
    }

    QSize size() const { return m_size; }
    void resize(const QSize &size);

private:
    QSize m_size{80, 60};
    std::shared_ptr<SharedData> m_data;
};

} // namespace Pty

class UTILS_EXPORT ProcessRunData
{
public:
    Utils::CommandLine command;
    Utils::FilePath workingDirectory;
    Utils::Environment environment = {};
};

class UTILS_EXPORT ProcessSetupData
{
public:
    ProcessMode m_processMode = ProcessMode::Reader;
    TerminalMode m_terminalMode = TerminalMode::Off;

    std::optional<Pty::Data> m_ptyData;
    CommandLine m_commandLine;
    FilePath m_workingDirectory;
    Environment m_environment;
    Environment m_controlEnvironment;
    QByteArray m_writeData;
    QProcess::ProcessChannelMode m_processChannelMode = QProcess::SeparateChannels;
    QVariantHash m_extraData;
    QString m_standardInputFile;
    QString m_nativeArguments; // internal, dependent on specific code path

    std::chrono::milliseconds m_reaperTimeout{500};
    bool m_abortOnMetaChars = true;
    bool m_runAsRoot = false;
    bool m_lowPriority = false;
    bool m_unixTerminalDisabled = false;
    bool m_useCtrlCStub = false;
    bool m_allowCoreDumps = true;
    bool m_belowNormalPriority = false; // internal, dependent on other fields and specific code path
    bool m_createConsoleOnWindows = false;
    bool m_forceDefaultErrorMode = false;
};

class UTILS_EXPORT ProcessResultData
{
public:
    int m_exitCode = 0;
    QProcess::ExitStatus m_exitStatus = QProcess::NormalExit;
    QProcess::ProcessError m_error = QProcess::UnknownError;
    QString m_errorString = {};
};

enum class ControlSignal { Terminate, Kill, Interrupt, KickOff, CloseWriteChannel };

enum class ProcessSignalType { Started, ReadyRead, Done };

class UTILS_EXPORT ProcessInterface : public QObject
{
    Q_OBJECT

signals:
    // This should be emitted when being in Starting state only.
    // After emitting this signal the process enters Running state.
    void started(qint64 processId, qint64 applicationMainThreadId = 0);

    // This should be emitted when being in Running state only.
    void readyRead(const QByteArray &outputData, const QByteArray &errorData);

    // This should be emitted when being in Starting or Running state.
    // When being in Starting state, the resultData should set error to FailedToStart.
    // After emitting this signal the process enters NotRunning state.
    void done(const ProcessResultData &resultData);

protected:
    static int controlSignalToInt(ControlSignal controlSignal);

    ProcessSetupData m_setup;

private:
    // It's being called only in Starting state. Just before this method is being called,
    // the process transitions from NotRunning into Starting state.
    virtual void start() = 0;

    // It's being called only in Running state.
    virtual qint64 write(const QByteArray &data) = 0;

    // It's being called in Starting or Running state.
    virtual void sendControlSignal(ControlSignal controlSignal) = 0;

    friend class Process;
    friend class Internal::ProcessPrivate;
};

} // namespace Utils
