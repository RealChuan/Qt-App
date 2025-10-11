// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "pluginmanager.h"

#include "extensionsystemtr.h"
#include "iplugin.h"
#include "optionsparser.h"
#include "pluginmanager_p.h"
#include "pluginspec.h"

#include <utils/algorithm.h>
#include <utils/benchmarker.h>
#include <utils/fileutils.h>
#include <utils/futuresynchronizer.h>
#include <utils/hostosinfo.h>
#include <utils/mimeutils.h>
#include <utils/qtcassert.h>
#include <utils/qtcprocess.h>
#include <utils/qtcsettings.h>
#include <utils/shutdownguard.h>
#include <utils/stringutils.h>
#include <utils/threadutils.h>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QGuiApplication>
#include <QLibrary>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QMetaProperty>
#include <QPluginLoader>
#include <QPushButton>
#include <QScopeGuard>
#include <QSysInfo>
#include <QTextStream>
#include <QTimer>
#include <QWriteLocker>

#ifdef EXTENSIONSYSTEM_WITH_TESTOPTION
#include <utils/hostosinfo.h>
#include <QTest>
#include <QThread>
#endif

#include <functional>
#include <memory>
#include <type_traits>

Q_LOGGING_CATEGORY(pluginLog, "qtc.extensionsystem", QtWarningMsg)

const char C_IGNORED_PLUGINS[] = "Plugins/Ignored";
const char C_FORCEENABLED_PLUGINS[] = "Plugins/ForceEnabled";
const char C_TANDCACCEPTED_PLUGINS[] = "Plugins/TermsAndConditionsAccepted";

const std::chrono::milliseconds DELAYED_INITIALIZE_INTERVAL{20};

enum { debugLeaks = 0 };

/*!
    \namespace ExtensionSystem
    \inmodule QtCreator
    \brief The ExtensionSystem namespace provides classes that belong to the
           core plugin system.

    The basic extension system contains the plugin manager and its supporting classes,
    and the IPlugin interface that must be implemented by plugin providers.
*/

/*!
    \namespace ExtensionSystem::Internal
    \internal
*/

/*!
    \class ExtensionSystem::PluginManager
    \inheaderfile extensionsystem/pluginmanager.h
    \inmodule QtCreator
    \ingroup mainclasses

    \brief The PluginManager class implements the core plugin system that
    manages the plugins, their life cycle, and their registered objects.

    The plugin manager is used for the following tasks:
    \list
    \li Manage plugins and their state
    \li Manipulate a \e {common object pool}
    \endlist

    \section1 Plugins
    Plugins must derive from the IPlugin class and have the IID
    \c "org.qt-project.Qt.QtCreatorPlugin".

    The plugin manager is used to set a list of file system directories to search for
    plugins, retrieve information about the state of these plugins, and to load them.

    Usually, the application creates a PluginManager instance and initiates the
    loading.
    \code
        // 'plugins' and subdirs will be searched for plugins
        PluginManager::setPluginPaths(QStringList("plugins"));
        PluginManager::loadPlugins(); // try to load all the plugins
    \endcode
    Additionally, it is possible to directly access plugin meta data, instances,
    and state.

    \section1 Object Pool
    Plugins (and everybody else) can add objects to a common \e pool that is located in
    the plugin manager. Objects in the pool must derive from QObject, there are no other
    prerequisites. Objects can be retrieved from the object pool via the getObject()
    and getObjectByName() functions.

    Whenever the state of the object pool changes, a corresponding signal is
    emitted by the plugin manager.

    A common usecase for the object pool is that a plugin (or the application) provides
    an \e {extension point} for other plugins, which is a class or interface that can
    be implemented and added to the object pool. The plugin that provides the
    extension point looks for implementations of the class or interface in the object pool.
    \code
        // Plugin A provides a "MimeTypeHandler" extension point
        // in plugin B:
        MyMimeTypeHandler *handler = new MyMimeTypeHandler();
        PluginManager::instance()->addObject(handler);
        // In plugin A:
        MimeTypeHandler *mimeHandler =
            PluginManager::getObject<MimeTypeHandler>();
    \endcode


    The ExtensionSystem::Invoker class template provides \e {syntactic sugar}
    for using \e soft extension points that may or may not be provided by an
    object in the pool. This approach neither requires the \e user plugin being
    linked against the \e provider plugin nor a common shared
    header file. The exposed interface is implicitly given by the
    invokable functions of the provider object in the object pool.

    The ExtensionSystem::invoke() function template encapsulates
    ExtensionSystem::Invoker construction for the common case where
    the success of the call is not checked.

    \code
        // In the "provide" plugin A:
        namespace PluginA {
        class SomeProvider : public QObject
        {
            Q_OBJECT

        public:
            Q_INVOKABLE QString doit(const QString &msg, int n) {
            {
                qDebug() << "I AM DOING IT " << msg;
                return QString::number(n);
            }
        };
        } // namespace PluginA


        // In the "user" plugin B:
        int someFuntionUsingPluginA()
        {
            using namespace ExtensionSystem;

            QObject *target = PluginManager::getObjectByClassName("PluginA::SomeProvider");

            if (target) {
                // Some random argument.
                QString msg = "REALLY.";

                // Plain function call, no return value.
                invoke<void>(target, "doit", msg, 2);

                // Plain function with no return value.
                qDebug() << "Result: " << invoke<QString>(target, "doit", msg, 21);

                // Record success of function call with return value.
                Invoker<QString> in1(target, "doit", msg, 21);
                qDebug() << "Success: (expected)" << in1.wasSuccessful();

                // Try to invoke a non-existing function.
                Invoker<QString> in2(target, "doitWrong", msg, 22);
                qDebug() << "Success (not expected):" << in2.wasSuccessful();

            } else {

                // We have to cope with plugin A's absence.
            }
        };
    \endcode

    \note The type of the parameters passed to the \c{invoke()} calls
    is deduced from the parameters themselves and must match the type of
    the arguments of the called functions \e{exactly}. No conversion or even
    integer promotions are applicable, so to invoke a function with a \c{long}
    parameter explicitly, use \c{long(43)} or such.

    \note The object pool manipulating functions are thread-safe.
*/

/*!
    \fn template <typename T> *ExtensionSystem::PluginManager::getObject()

    Retrieves the object of a given type from the object pool.

    This function uses \c qobject_cast to determine the type of an object.
    If there are more than one objects of the given type in
    the object pool, this function will arbitrarily choose one of them.

    \sa addObject()
*/

/*!
    \fn template <typename T, typename Predicate> *ExtensionSystem::PluginManager::getObject(Predicate predicate)

    Retrieves the object of a given type from the object pool that matches
    the \a predicate.

    This function uses \c qobject_cast to determine the type of an object.
    The predicate must be a function taking T * and returning a bool.
    If there is more than one object matching the type and predicate,
    this function will arbitrarily choose one of them.

    \sa addObject()
*/

using namespace Utils;

static void registerMimeFromPlugin(const ExtensionSystem::PluginSpec *plugin)
{
    const QJsonObject metaData = plugin->metaData();
    const QJsonValue mimetypes = metaData.value("Mimetypes");
    QString mimetypeString;
    if (Utils::readMultiLineString(mimetypes, &mimetypeString))
        Utils::addMimeTypes(plugin->id() + ".mimetypes", mimetypeString.trimmed().toUtf8());
}

namespace ExtensionSystem {

using namespace Internal;

static Internal::PluginManagerPrivate *d = nullptr;
static PluginManager *m_instance = nullptr;

/*!
    Gets the unique plugin manager instance.
*/
PluginManager *PluginManager::instance()
{
    return m_instance;
}

/*!
    Creates a plugin manager. Should be done only once per application.
*/
PluginManager::PluginManager()
{
    m_instance = this;
    d = new PluginManagerPrivate(this);
    shutdownGuard(); // ensure creation on main thread
}

/*!
    \internal
*/
PluginManager::~PluginManager()
{
    delete d;
    d = nullptr;
}

/*!
    Adds the object \a obj to the object pool, so it can be retrieved
    again from the pool by type.

    The plugin manager does not do any memory management. Added objects
    must be removed from the pool and deleted manually by whoever is responsible for the object.

    Emits the \c objectAdded() signal.

    \sa PluginManager::removeObject()
    \sa PluginManager::getObject()
    \sa PluginManager::getObjectByName()
*/
void PluginManager::addObject(QObject *obj)
{
    d->addObject(obj);
}

/*!
    Emits the \c aboutToRemoveObject() signal and removes the object \a obj
    from the object pool.
    \sa PluginManager::addObject()
*/
void PluginManager::removeObject(QObject *obj)
{
    d->removeObject(obj);
}

/*!
    Retrieves the list of all objects in the pool, unfiltered.

    Usually, clients do not need to call this function.

    \sa PluginManager::getObject()
*/
QObjectList PluginManager::allObjects()
{
    return d->allObjects;
}

/*!
    \internal
*/
QReadWriteLock *PluginManager::listLock()
{
    return &d->m_lock;
}

/*!
    Tries to load all the plugins that were previously found when
    setting the plugin search paths. The plugin specs of the plugins
    can be used to retrieve error and state information about individual plugins.

    \sa setPluginPaths()
    \sa plugins()
*/
void PluginManager::loadPlugins()
{
    d->loadPlugins();
}

void PluginManager::loadPluginsAtRuntime(const QSet<PluginSpec *> &plugins)
{
    d->loadPluginsAtRuntime(plugins);
}

void PluginManager::addPlugins(const PluginSpecs &specs)
{
    d->addPlugins(specs);
}

void PluginManager::removePluginsAfterRestart()
{
    d->removePluginsAfterRestart();
}

void PluginManager::installPluginsAfterRestart()
{
    d->installPluginsAfterRestart();
}

Result<> PluginManager::removePluginOnRestart(const QString &id)
{
    return d->removePluginOnRestart(id);
}

void PluginManager::installPluginOnRestart(const Utils::FilePath &source,
                                           const Utils::FilePath &destination)
{
    d->installPluginOnRestart(source, destination);
}

/*!
    Returns \c true if any plugin has errors even though it is enabled.
    Most useful to call after loadPlugins().
*/
bool PluginManager::hasError()
{
    return Utils::anyOf(plugins(), [](PluginSpec *spec) {
        // only show errors on startup if plugin is enabled.
        return spec->hasError() && spec->isEffectivelyEnabled();
    });
}

const QStringList PluginManager::allErrors()
{
    return Utils::transform<QStringList>(Utils::filtered(plugins(),
                                                         [](const PluginSpec *spec) {
                                                             return spec->hasError()
                                                                    && spec->isEffectivelyEnabled();
                                                         }),
                                         [](const PluginSpec *spec) {
                                             return spec->id().append(": ").append(
                                                 spec->errorString());
                                         });
}

/*!
    Returns all plugins that require \a spec to be loaded. Recurses into dependencies.
 */
const QSet<PluginSpec *> PluginManager::pluginsRequiringPlugin(PluginSpec *spec)
{
    QSet<PluginSpec *> dependingPlugins({spec});
    // recursively add plugins that depend on plugins that.... that depend on spec
    for (PluginSpec *spec : d->loadQueue()) {
        if (spec->requiresAny(dependingPlugins))
            dependingPlugins.insert(spec);
    }
    dependingPlugins.remove(spec);
    return dependingPlugins;
}

/*!
    Returns all plugins that \a spec requires to be loaded. Recurses into dependencies.
 */
const QSet<PluginSpec *> PluginManager::pluginsToEnableForPlugin(PluginSpec *spec)
{
    QSet<PluginSpec *> recursiveDependencies;
    recursiveDependencies.insert(spec);
    std::queue<PluginSpec *> queue;
    queue.push(spec);
    while (!queue.empty()) {
        PluginSpec *checkSpec = queue.front();
        queue.pop();
        // add dependencies
        const QHash<PluginDependency, PluginSpec *> deps = checkSpec->dependencySpecs();
        for (auto depIt = deps.cbegin(), end = deps.cend(); depIt != end; ++depIt) {
            if (depIt.key().type != PluginDependency::Required)
                continue;
            PluginSpec *depSpec = depIt.value();
            if (Utils::insert(recursiveDependencies, depSpec))
                queue.push(depSpec);
        }
        // add recommended plugins
        // (e.g. when enabling qmldesigner, also enable isoiconbrowser)
        const QSet<PluginSpec *> recommends = checkSpec->recommendsSpecs();
        for (PluginSpec *rec : recommends) {
            if (Utils::insert(recursiveDependencies, rec))
                queue.push(rec);
        }
    }
    recursiveDependencies.remove(spec);
    return recursiveDependencies;
}

/*!
    Shuts down and deletes all plugins.
*/
void PluginManager::shutdown()
{
    d->shutdown();
}

static QString filled(const QString &s, int min)
{
    return s + QString(qMax(0, min - s.size()), ' ');
}

QString PluginManager::systemInformation()
{
    QString result;
    CommandLine qtDiag(FilePath::fromString(QLibraryInfo::path(QLibraryInfo::BinariesPath))
                           .pathAppended("qtdiag")
                           .withExecutableSuffix());
    Process qtDiagProc;
    qtDiagProc.setCommand(qtDiag);
    qtDiagProc.runBlocking();
    if (qtDiagProc.result() == ProcessResult::FinishedWithSuccess)
        result += qtDiagProc.allOutput() + "\n";
    result += "Plugin information:\n\n";
    PluginSpec *const longestSpec = Utils::maxElementOrDefault(d->pluginSpecs,
                                                               [](const PluginSpec *left,
                                                                  const PluginSpec *right) {
                                                                   return left->id().size()
                                                                          < right->id().size();
                                                               });
    int size = longestSpec->id().size();
    for (const PluginSpec *spec : plugins()) {
        result += QLatin1String(spec->isEffectivelyEnabled() ? "+ " : "  ")
                  + filled(spec->id(), size) + " " + spec->version() + "\n";
    }
    QString settingspath = QFileInfo(settings()->fileName()).path();
    if (settingspath.startsWith(QDir::homePath()))
        settingspath.replace(QDir::homePath(), "~");
    result += "\nUsed settingspath: " + settingspath + "\n";
    return result;
}

/*!
    The list of paths were the plugin manager searches for plugins.

    \sa setPluginPaths()
*/
FilePaths PluginManager::pluginPaths()
{
    return d->pluginPaths;
}

/*!
    Sets the plugin paths. All the specified \a paths and their subdirectory
    trees are searched for plugins.

    \sa pluginPaths()
    \sa loadPlugins()
*/
void PluginManager::setPluginPaths(const FilePaths &paths)
{
    d->setPluginPaths(paths);
}

/*!
    The IID that valid plugins must have.

    \sa setPluginIID()
*/
QString PluginManager::pluginIID()
{
    return d->pluginIID;
}

/*!
    Sets the IID that valid plugins must have to \a iid. Only plugins with this
    IID are loaded, others are silently ignored.

    At the moment this must be called before setPluginPaths() is called.

    \omit
    // ### TODO let this + setPluginPaths read the plugin meta data lazyly whenever loadPlugins() or plugins() is called.
    \endomit
*/
void PluginManager::setPluginIID(const QString &iid)
{
    d->pluginIID = iid;
}

/*!
    Returns the user specific settings used for information about enabled and
    disabled plugins.
*/
QtcSettings *PluginManager::settings()
{
    return &Utils::userSettings();
}

/*!
    Returns the global (user-independent) settings used for information about default disabled plugins.
*/
QtcSettings *PluginManager::globalSettings()
{
    return &Utils::installSettings();
}

void PluginManager::writeSettings()
{
    d->writeSettings();
}

/*!
    The arguments left over after parsing (that were neither startup nor plugin
    arguments). Typically, this will be the list of files to open.
*/
QStringList PluginManager::arguments()
{
    return d->arguments;
}

/*!
    The arguments that should be used when automatically restarting the application.
    This includes plugin manager related options for enabling or disabling plugins,
    but excludes others, like the arguments returned by arguments() and the appOptions
    passed to the parseOptions() method.
*/
QStringList PluginManager::argumentsForRestart()
{
    return d->argumentsForRestart;
}

/*!
    List of all plugins that have been found in the plugin search paths.
    This list is valid directly after the setPluginPaths() call.
    The plugin specifications contain plugin metadata and the current state
    of the plugins. If a plugin's library has been already successfully loaded,
    the plugin specification has a reference to the created plugin instance as well.

    \sa setPluginPaths()
*/
const PluginSpecs PluginManager::plugins()
{
    return d->pluginSpecs;
}

QHash<QString, PluginSpecs> PluginManager::pluginCollections()
{
    return d->pluginCategories;
}

static const char argumentKeywordC[] = ":arguments";
static const char pwdKeywordC[] = ":pwd";

/*!
    Serializes plugin options and arguments for sending in a single string
    via QtSingleApplication:
    ":myplugin|-option1|-option2|:arguments|argument1|argument2",
    as a list of lists started by a keyword with a colon. Arguments are last.

    \sa setPluginPaths()
*/
QString PluginManager::serializedArguments()
{
    const QChar separator = QLatin1Char('|');
    QString rc;
    for (const PluginSpec *ps : plugins()) {
        if (!ps->arguments().isEmpty()) {
            if (!rc.isEmpty())
                rc += separator;
            rc += QLatin1Char(':');
            rc += ps->id();
            rc += separator;
            rc += ps->arguments().join(separator);
        }
    }
    if (!rc.isEmpty())
        rc += separator;
    rc += QLatin1String(pwdKeywordC) + separator + QDir::currentPath();
    if (!d->arguments.isEmpty()) {
        if (!rc.isEmpty())
            rc += separator;
        rc += QLatin1String(argumentKeywordC);
        for (const QString &argument : std::as_const(d->arguments))
            rc += separator + argument;
    }
    return rc;
}

/* Extract a sublist from the serialized arguments
 * indicated by a keyword starting with a colon indicator:
 * ":a,i1,i2,:b:i3,i4" with ":a" -> "i1,i2"
 */
static QStringList subList(const QStringList &in, const QString &key)
{
    QStringList rc;
    // Find keyword and copy arguments until end or next keyword
    const QStringList::const_iterator inEnd = in.constEnd();
    QStringList::const_iterator it = std::find(in.constBegin(), inEnd, key);
    if (it != inEnd) {
        const QChar nextIndicator = QLatin1Char(':');
        for (++it; it != inEnd && !it->startsWith(nextIndicator); ++it)
            rc.append(*it);
    }
    return rc;
}

/*!
    Parses the options encoded in \a serializedArgument
    and passes them on to the respective plugins along with the arguments.

    \a socket is passed for disconnecting the peer when the operation is done (for example,
    document is closed) for supporting the \c -block flag.
*/

void PluginManager::remoteArguments(const QString &serializedArgument, QObject *socket)
{
    if (isShuttingDown())
        return;
    if (serializedArgument.isEmpty())
        return;
    QStringList serializedArguments = serializedArgument.split(QLatin1Char('|'));
    const QStringList pwdValue = subList(serializedArguments, QLatin1String(pwdKeywordC));
    const QString workingDirectory = pwdValue.isEmpty() ? QString() : pwdValue.first();
    const QStringList arguments = subList(serializedArguments, QLatin1String(argumentKeywordC));
    for (const PluginSpec *ps : plugins()) {
        if (ps->state() == PluginSpec::Running) {
            const QStringList pluginOptions = subList(serializedArguments,
                                                      QLatin1Char(':') + ps->id());
            if (IPlugin *plugin = ps->plugin()) {
                QObject *socketParent = plugin->remoteCommand(pluginOptions,
                                                              workingDirectory,
                                                              arguments);
                if (socketParent && socket) {
                    socket->setParent(socketParent);
                    socket = nullptr;
                }
            }
        }
    }
    if (socket)
        delete socket;
}

/*!
    Takes the list of command line options in \a args and parses them.
    The plugin manager itself might process some options itself directly
    (\c {-noload <plugin>}), and adds options that are registered by
    plugins to their plugin specs.

    The caller (the application) may register itself for options via the
    \a appOptions list, containing pairs of \e {option string} and a bool
    that indicates whether the option requires an argument.
    Application options always override any plugin's options.

    \a foundAppOptions is set to pairs of (\e {option string}, \e argument)
    for any application options that were found.
    The command line options that were not processed can be retrieved via the arguments() function.

    Returns if there was an error.
 */
Result<> PluginManager::parseOptions(const QStringList &args,
                                     const QMap<QString, bool> &appOptions,
                                     QMap<QString, QString> *foundAppOptions)
{
    OptionsParser options(args, appOptions, foundAppOptions, d);
    return options.parse();
}

static inline void indent(QTextStream &str, int indent)
{
    str << QString(indent, ' ');
}

static inline void formatOption(QTextStream &str,
                                const QString &opt,
                                const QString &parm,
                                const QString &description,
                                int optionIndentation,
                                int descriptionIndentation)
{
    int remainingIndent = descriptionIndentation - optionIndentation - opt.size();
    indent(str, optionIndentation);
    str << opt;
    if (!parm.isEmpty()) {
        str << " <" << parm << '>';
        remainingIndent -= 3 + parm.size();
    }
    if (remainingIndent >= 1) {
        indent(str, remainingIndent);
    } else {
        str << '\n';
        indent(str, descriptionIndentation);
    }
    str << description << '\n';
}

/*!
    Formats the startup options of the plugin manager for command line help with the specified
    \a optionIndentation and \a descriptionIndentation.
    Adds the result to \a str.
*/

void PluginManager::formatOptions(QTextStream &str,
                                  int optionIndentation,
                                  int descriptionIndentation)
{
    formatOption(str,
                 QLatin1String(OptionsParser::LOAD_OPTION),
                 QLatin1String("plugin"),
                 QLatin1String("Load <plugin> and all plugins that it requires"),
                 optionIndentation,
                 descriptionIndentation);
    formatOption(str,
                 QLatin1String(OptionsParser::LOAD_OPTION) + QLatin1String(" all"),
                 QString(),
                 QLatin1String("Load all available plugins"),
                 optionIndentation,
                 descriptionIndentation);
    formatOption(str,
                 QLatin1String(OptionsParser::NO_LOAD_OPTION),
                 QLatin1String("plugin"),
                 QLatin1String("Do not load <plugin> and all plugins that require it"),
                 optionIndentation,
                 descriptionIndentation);
    formatOption(str,
                 QLatin1String(OptionsParser::NO_LOAD_OPTION) + QLatin1String(" all"),
                 QString(),
                 QString::fromLatin1("Do not load any plugin (useful when "
                                     "followed by one or more \"%1\" arguments)")
                     .arg(QLatin1String(OptionsParser::LOAD_OPTION)),
                 optionIndentation,
                 descriptionIndentation);
    formatOption(str,
                 QLatin1String(OptionsParser::PROFILE_OPTION),
                 QString(),
                 QLatin1String("Profile plugin loading"),
                 optionIndentation,
                 descriptionIndentation);
    formatOption(str,
                 QLatin1String(OptionsParser::TRACE_OPTION),
                 QLatin1String("file"),
                 QLatin1String("Write trace file (CTF) for plugin loading"),
                 optionIndentation,
                 descriptionIndentation);
    formatOption(str,
                 QLatin1String(OptionsParser::NO_CRASHCHECK_OPTION),
                 QString(),
                 QLatin1String("Disable startup check for previously crashed instance"),
                 optionIndentation,
                 descriptionIndentation);
#ifdef EXTENSIONSYSTEM_WITH_TESTOPTION
    formatOption(str,
                 QString::fromLatin1(OptionsParser::TEST_OPTION)
                     + QLatin1String(" <plugin>[,testfunction[:testdata]]..."),
                 QString(),
                 QLatin1String("Run plugin's tests (by default a separate settings path is used)"),
                 optionIndentation,
                 descriptionIndentation);
    formatOption(str,
                 QString::fromLatin1(OptionsParser::TEST_OPTION) + QLatin1String(" all"),
                 QString(),
                 QLatin1String("Run tests from all plugins"),
                 optionIndentation,
                 descriptionIndentation);
    formatOption(str,
                 QString::fromLatin1(OptionsParser::NOTEST_OPTION),
                 QLatin1String("plugin"),
                 QLatin1String("Exclude all of the plugin's tests from the test run"),
                 optionIndentation,
                 descriptionIndentation);
    formatOption(str,
                 QString::fromLatin1(OptionsParser::SCENARIO_OPTION),
                 QString("scenarioname"),
                 QLatin1String("Run given scenario"),
                 optionIndentation,
                 descriptionIndentation);
#endif
}

/*!
    Formats the plugin options of the plugin specs for command line help with the specified
    \a optionIndentation and \a descriptionIndentation.
    Adds the result to \a str.
*/

void PluginManager::formatPluginOptions(QTextStream &str,
                                        int optionIndentation,
                                        int descriptionIndentation)
{
    // Check plugins for options
    for (PluginSpec *ps : std::as_const(d->pluginSpecs)) {
        const PluginSpec::PluginArgumentDescriptions pargs = ps->argumentDescriptions();
        if (!pargs.empty()) {
            str << "\nPlugin: " << ps->id() << '\n';
            for (const PluginArgumentDescription &pad : pargs)
                formatOption(str,
                             pad.name,
                             pad.parameter,
                             pad.description,
                             optionIndentation,
                             descriptionIndentation);
        }
    }
}

/*!
    Formats the version of the plugin specs for command line help and adds it to \a str.
*/
void PluginManager::formatPluginVersions(QTextStream &str)
{
    for (PluginSpec *ps : std::as_const(d->pluginSpecs))
        str << "  " << ps->id() << ' ' << ps->version() << ' ' << ps->description() << '\n';
}

/*!
    \internal
 */
bool PluginManager::testRunRequested()
{
    return !d->testSpecs.empty();
}

#ifdef EXTENSIONSYSTEM_WITH_TESTOPTION
// Called in plugin initialization, the scenario function will be called later, from main
bool PluginManager::registerScenario(const QString &scenarioId,
                                     std::function<bool()> scenarioStarter)
{
    if (d->m_scenarios.contains(scenarioId)) {
        const QString warning = QString("Can't register scenario \"%1\" as the other scenario was "
                                        "already registered with this name.")
                                    .arg(scenarioId);
        qWarning("%s", qPrintable(warning));
        return false;
    }

    d->m_scenarios.insert(scenarioId, scenarioStarter);
    return true;
}

// Called from main
bool PluginManager::isScenarioRequested()
{
    return !d->m_requestedScenario.isEmpty();
}

// Called from main (may be squashed with the isScenarioRequested: runScenarioIfRequested).
// Returns false if scenario couldn't run (e.g. no Qt version set)
bool PluginManager::runScenario()
{
    if (d->m_isScenarioRunning) {
        qWarning("Scenario is already running. Can't run scenario recursively.");
        return false;
    }

    if (d->m_requestedScenario.isEmpty()) {
        qWarning("Can't run any scenario since no scenario was requested.");
        return false;
    }

    if (!d->m_scenarios.contains(d->m_requestedScenario)) {
        const QString warning = QString("Requested scenario \"%1\" was not registered.")
                                    .arg(d->m_requestedScenario);
        qWarning("%s", qPrintable(warning));
        return false;
    }

    d->m_isScenarioRunning = true;
    // The return value comes now from scenarioStarted() function. It may fail e.g. when
    // no Qt version is set. Initializing the scenario may take some time, that's why
    // waitForScenarioFullyInitialized() was added.
    bool ret = d->m_scenarios[d->m_requestedScenario]();

    QMutexLocker locker(&d->m_scenarioMutex);
    d->m_scenarioFullyInitialized = true;
    d->m_scenarioWaitCondition.wakeAll();

    return ret;
}

// Called from scenario point (and also from runScenario - don't run scenarios recursively).
// This may be called from non-main thread. We assume that m_requestedScenario
// may only be changed from the main thread.
bool PluginManager::isScenarioRunning(const QString &scenarioId)
{
    return d->m_isScenarioRunning && d->m_requestedScenario == scenarioId;
}

// This may be called from non-main thread.
bool PluginManager::finishScenario()
{
    if (!d->m_isScenarioRunning)
        return false; // Can't finish not running scenario

    if (d->m_isScenarioFinished.exchange(true))
        return false; // Finish was already called before. We return false, as we didn't finish it right now.

    QMetaObject::invokeMethod(d, [] { emit m_instance->scenarioFinished(0); });
    return true; // Finished successfully.
}

// Waits until the running scenario is fully initialized
void PluginManager::waitForScenarioFullyInitialized()
{
    if (isMainThread()) {
        qWarning(
            "The waitForScenarioFullyInitialized() function can't be called from main thread.");
        return;
    }
    QMutexLocker locker(&d->m_scenarioMutex);
    if (d->m_scenarioFullyInitialized)
        return;

    d->m_scenarioWaitCondition.wait(&d->m_scenarioMutex);
}
#endif

void PluginManager::setCreatorProcessData(const PluginManager::ProcessData &data)
{
    d->m_creatorProcessData = data;
}

PluginManager::ProcessData PluginManager::creatorProcessData()
{
    return d->m_creatorProcessData;
}

/*!
    Returns a list of plugins in load order.
*/
PluginSpecs PluginManager::loadQueue()
{
    // Ensure order preservation
    static_assert(std::is_same<PluginSpecs, QList<class PluginSpec *>>::value);
    return d->loadQueue();
}

//============PluginManagerPrivate===========

void PluginManagerPrivate::startDelayedInitialize()
{
    Utils::setMimeStartupPhase(MimeStartupPhase::PluginsDelayedInitializing);
    {
        while (!delayedInitializeQueue.empty()) {
            PluginSpec *spec = delayedInitializeQueue.front();
            const std::string specName = spec->id().toStdString();
            delayedInitializeQueue.pop();
            profilingReport(">delayedInitialize", spec);
            bool delay = spec->delayedInitialize();
            profilingReport("<delayedInitialize", spec, &spec->performanceData().delayedInitialize);
            if (delay) // give UI a bit of breathing space, but prevent user interaction
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        Utils::setMimeStartupPhase(MimeStartupPhase::UpAndRunning);
        m_isInitializationDone = true;
        if (m_profileTimer)
            m_totalStartupMS = m_profileTimer->elapsed();
        printProfilingSummary();
    }
    emit q->initializationDone();
#ifdef EXTENSIONSYSTEM_WITH_TESTOPTION
    if (PluginManager::testRunRequested())
        startTests();
    else if (PluginManager::isScenarioRequested()) {
        if (PluginManager::runScenario()) {
            const QString info = QString("Successfully started scenario \"%1\"...")
                                     .arg(d->m_requestedScenario);
            qInfo("%s", qPrintable(info));
        } else {
            QMetaObject::invokeMethod(this, [] { emit m_instance->scenarioFinished(1); });
        }
    }
#endif
}

/*!
    \internal
*/
PluginManagerPrivate::PluginManagerPrivate(PluginManager *pluginManager)
    : q(pluginManager)
{}

/*!
    \internal
*/
PluginManagerPrivate::~PluginManagerPrivate()
{
    qDeleteAll(pluginSpecs);
}

/*!
    \internal
*/
void PluginManagerPrivate::writeSettings()
{
    QtcSettings &settings = Utils::userSettings();
    QStringList tempDisabledPlugins;
    QStringList tempForceEnabledPlugins;
    for (PluginSpec *spec : std::as_const(pluginSpecs)) {
        if (spec->isEnabledByDefault() && !spec->isEnabledBySettings())
            tempDisabledPlugins.append(spec->id());
        if (!spec->isEnabledByDefault() && spec->isEnabledBySettings())
            tempForceEnabledPlugins.append(spec->id());
    }

    settings.setValueWithDefault(C_IGNORED_PLUGINS, tempDisabledPlugins);
    settings.setValueWithDefault(C_FORCEENABLED_PLUGINS, tempForceEnabledPlugins);
}

static inline QStringList toLower(const QStringList &list)
{
    return Utils::transform(list, [](const QString &s) { return s.toLower(); });
}

/*!
    \internal
*/
void PluginManagerPrivate::readSettings()
{
    QtcSettings &userSettings = Utils::userSettings();
    QtcSettings &globalSettings = Utils::installSettings();

    defaultDisabledPlugins = toLower(globalSettings.value(C_IGNORED_PLUGINS).toStringList());
    defaultEnabledPlugins = toLower(globalSettings.value(C_FORCEENABLED_PLUGINS).toStringList());

    disabledPlugins = toLower(userSettings.value(C_IGNORED_PLUGINS).toStringList());
    forceEnabledPlugins = toLower(userSettings.value(C_FORCEENABLED_PLUGINS).toStringList());
    pluginsWithAcceptedTermsAndConditions = filteredUnique(
        userSettings.value(C_TANDCACCEPTED_PLUGINS).toStringList());
}

/*!
    \internal
*/
void PluginManagerPrivate::stopAll()
{
    m_isShuttingDown = true;
    delayedInitializeTimer.stop();

    const PluginSpecs queue = loadQueue();
    for (PluginSpec *spec : queue)
        loadPlugin(spec, PluginSpec::Stopped);
}

/*!
    \internal
*/
void PluginManagerPrivate::deleteAll()
{
    // Guard against someone playing with the setting
    QTC_ASSERT(Utils::futureSynchronizer()->isCancelOnWait(),
               Utils::futureSynchronizer()->cancelAllFutures());
    Utils::futureSynchronizer()->waitForFinished(); // Synchronize all futures from all plugins
    triggerShutdownGuard();
    Utils::reverseForeach(loadQueue(),
                          [this](PluginSpec *spec) { loadPlugin(spec, PluginSpec::Deleted); });
}

void PluginManagerPrivate::checkForDuplicatePlugins()
{
    QHash<QString, PluginSpec *> seen;
    for (PluginSpec *spec : std::as_const(pluginSpecs)) {
        if (PluginSpec *other = seen.value(spec->id())) {
            // Plugin with same name already there. We do not know, which version is the right one,
            // keep it simple and fail both (if enabled).
            if (spec->isEffectivelyEnabled() && other->isEffectivelyEnabled()) {
                const QString error = Tr::tr(
                    "Multiple versions of the same plugin have been found.");
                spec->setError(error);
                other->setError(error);
            }
        } else {
            if (!spec->id().isEmpty())
                seen.insert(spec->id(), spec);
        }
    }
}

static QHash<IPlugin *, QList<TestCreator>> g_testCreators;

void PluginManagerPrivate::addTestCreator([[maybe_unused]] IPlugin *plugin,
                                          [[maybe_unused]] const TestCreator &testCreator)
{
#ifdef EXTENSIONSYSTEM_WITH_TESTOPTION
    g_testCreators[plugin].append(testCreator);
#endif
}

#ifdef EXTENSIONSYSTEM_WITH_TESTOPTION

using TestPlan = QHash<QObject *, QStringList>; // Object -> selected test functions

static bool isTestFunction(const QMetaMethod &metaMethod)
{
    static const QList<QByteArray> blackList = {"initTestCase()",
                                                "cleanupTestCase()",
                                                "init()",
                                                "cleanup()"};

    if (metaMethod.methodType() != QMetaMethod::Slot)
        return false;

    if (metaMethod.access() != QMetaMethod::Private)
        return false;

    const QByteArray signature = metaMethod.methodSignature();
    if (blackList.contains(signature))
        return false;

    if (!signature.startsWith("test"))
        return false;

    if (signature.endsWith("_data()"))
        return false;

    return true;
}

static QStringList testFunctions(const QMetaObject *metaObject)
{
    if (!metaObject)
        return {};

    QStringList functions;
    for (int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i) {
        const QMetaMethod metaMethod = metaObject->method(i);
        if (isTestFunction(metaMethod)) {
            const QByteArray signature = metaMethod.methodSignature();
            const QString method = QString::fromLatin1(signature);
            const QString methodName = method.left(method.size() - 2);
            functions.append(methodName);
        }
    }

    return testFunctions(metaObject->superClass()) + functions;
}

static QStringList matchingTestFunctions(const QStringList &testFunctions, const QString &matchText)
{
    // There might be a test data suffix like in "testfunction:testdata1".
    QString testFunctionName = matchText;
    QString testDataSuffix;
    const int index = testFunctionName.indexOf(QLatin1Char(':'));
    if (index != -1) {
        testDataSuffix = testFunctionName.mid(index);
        testFunctionName = testFunctionName.left(index);
    }

    const QRegularExpression regExp(
        QRegularExpression::wildcardToRegularExpression(testFunctionName));
    QStringList matchingFunctions;
    for (const QString &testFunction : testFunctions) {
        if (regExp.match(testFunction).hasMatch()) {
            // If the specified test data is invalid, the QTest framework will
            // print a reasonable error message for us.
            matchingFunctions.append(testFunction + testDataSuffix);
        }
    }

    return matchingFunctions;
}

static QObject *objectWithClassName(const QObjectList &objects, const QString &className)
{
    return Utils::findOr(objects, nullptr, [className](QObject *object) -> bool {
        QString candidate = object->objectName();
        if (candidate.isEmpty())
            candidate = QString::fromUtf8(object->metaObject()->className());
        const int colonIndex = candidate.lastIndexOf(QLatin1Char(':'));
        if (colonIndex != -1 && colonIndex < candidate.size() - 1)
            candidate = candidate.mid(colonIndex + 1);
        return candidate == className;
    });
}

static int executeTestPlan(const TestPlan &testPlan)
{
    int failedTests = 0;

    for (auto it = testPlan.cbegin(), end = testPlan.cend(); it != end; ++it) {
        QObject *testObject = it.key();
        QStringList functions = it.value();

        // Don't run QTest::qExec without any test functions, that'd run *all* slots as tests.
        if (functions.isEmpty())
            continue;

        functions.removeDuplicates();

        // QTest::qExec() expects basically QCoreApplication::arguments(),
        QStringList qExecArguments = QStringList() << QLatin1String("arg0") // fake application name
                                                   << QLatin1String("-maxwarnings")
                                                   << QLatin1String("0"); // unlimit output
        qExecArguments << functions;
        // avoid being stuck in QTBUG-24925
        if (!HostOsInfo::isWindowsHost())
            qExecArguments << "-nocrashhandler";
        failedTests += QTest::qExec(testObject, qExecArguments);
    }

    return failedTests;
}

/// Resulting plan consists of all test functions of the plugin object and
/// all test functions of all test objects of the plugin.
static TestPlan generateCompleteTestPlan(IPlugin *plugin, const QObjectList &testObjects)
{
    TestPlan testPlan;

    testPlan.insert(plugin, testFunctions(plugin->metaObject()));
    for (QObject *testObject : testObjects) {
        const QStringList allFunctions = testFunctions(testObject->metaObject());
        testPlan.insert(testObject, allFunctions);
    }

    return testPlan;
}

/// Resulting plan consists of all matching test functions of the plugin object
/// and all matching functions of all test objects of the plugin. However, if a
/// match text denotes a test class, all test functions of that will be
/// included and the class will not be considered further.
///
/// Since multiple match texts can match the same function, a test function might
/// be included multiple times for a test object.
static TestPlan generateCustomTestPlan(IPlugin *plugin,
                                       const QObjectList &testObjects,
                                       const QStringList &matchTexts)
{
    TestPlan testPlan;

    const QStringList testFunctionsOfPluginObject = testFunctions(plugin->metaObject());
    QStringList matchedTestFunctionsOfPluginObject;
    QStringList remainingMatchTexts = matchTexts;
    QObjectList remainingTestObjectsOfPlugin = testObjects;

    while (!remainingMatchTexts.isEmpty()) {
        const QString matchText = remainingMatchTexts.takeFirst();
        bool matched = false;

        if (QObject *testObject = objectWithClassName(remainingTestObjectsOfPlugin, matchText)) {
            // Add all functions of the matching test object
            matched = true;
            testPlan.insert(testObject, testFunctions(testObject->metaObject()));
            remainingTestObjectsOfPlugin.removeAll(testObject);

        } else {
            // Add all matching test functions of all remaining test objects
            for (QObject *testObject : std::as_const(remainingTestObjectsOfPlugin)) {
                const QStringList allFunctions = testFunctions(testObject->metaObject());
                const QStringList matchingFunctions = matchingTestFunctions(allFunctions, matchText);
                if (!matchingFunctions.isEmpty()) {
                    matched = true;
                    testPlan[testObject] += matchingFunctions;
                }
            }
        }

        const QStringList currentMatchedTestFunctionsOfPluginObject
            = matchingTestFunctions(testFunctionsOfPluginObject, matchText);
        if (!currentMatchedTestFunctionsOfPluginObject.isEmpty()) {
            matched = true;
            matchedTestFunctionsOfPluginObject += currentMatchedTestFunctionsOfPluginObject;
        }

        if (!matched) {
            QTextStream out(stdout);
            out << "No test function or class matches \"" << matchText << "\" in plugin \""
                << plugin->metaObject()->className() << "\".\nAvailable functions:\n";
            for (const QString &f : testFunctionsOfPluginObject)
                out << "  " << f << '\n';
            out << '\n';
        }
    }

    // Add all matching test functions of plugin
    if (!matchedTestFunctionsOfPluginObject.isEmpty())
        testPlan.insert(plugin, matchedTestFunctionsOfPluginObject);

    return testPlan;
}

void PluginManagerPrivate::startTests()
{
    if (PluginManager::hasError()) {
        qWarning("Errors occurred while loading plugins, skipping test run.");
        for (const QString &pluginError : PluginManager::allErrors())
            qWarning("%s", qPrintable(pluginError));
        QTimer::singleShot(1, QCoreApplication::instance(), &QCoreApplication::quit);
        return;
    }

    int failedTests = 0;
    for (const TestSpec &testSpec : std::as_const(testSpecs)) {
        IPlugin *plugin = testSpec.pluginSpec->plugin();
        if (!plugin)
            continue; // plugin not loaded

        const QList<TestCreator> testCreators = g_testCreators[plugin];
        const QObjectList testObjects = Utils::transform(testCreators, &TestCreator::operator());
        const QScopeGuard cleanup([&] { qDeleteAll(testObjects); });

        const bool hasDuplicateTestObjects = testObjects.size()
                                             != Utils::filteredUnique(testObjects).size();
        QTC_ASSERT(!hasDuplicateTestObjects, continue);
        QTC_ASSERT(!testObjects.contains(plugin), continue);

        const TestPlan testPlan = testSpec.testFunctionsOrObjects.isEmpty()
                                      ? generateCompleteTestPlan(plugin, testObjects)
                                      : generateCustomTestPlan(plugin,
                                                               testObjects,
                                                               testSpec.testFunctionsOrObjects);

        failedTests += executeTestPlan(testPlan);
    }

    QTimer::singleShot(0, this, [failedTests] { emit m_instance->testsFinished(failedTests); });
}
#endif

/*!
    \internal
*/
void PluginManagerPrivate::addObject(QObject *obj)
{
    {
        QWriteLocker lock(&m_lock);
        if (obj == nullptr) {
            qWarning() << "PluginManagerPrivate::addObject(): trying to add null object";
            return;
        }
        if (allObjects.contains(obj)) {
            qWarning() << "PluginManagerPrivate::addObject(): trying to add duplicate object";
            return;
        }

        if (debugLeaks)
            qDebug() << "PluginManagerPrivate::addObject" << obj << obj->objectName();

        if (m_profilingVerbosity > 1 && m_profileTimer) {
            // Report a timestamp when adding an object. Useful for profiling
            // its initialization time.
            const int absoluteElapsedMS = int(m_profileTimer->elapsed());
            qDebug("  %-43s %8dms", obj->metaObject()->className(), absoluteElapsedMS);
        }

        allObjects.append(obj);
    }
    emit q->objectAdded(obj);
}

/*!
    \internal
*/
void PluginManagerPrivate::removeObject(QObject *obj)
{
    if (obj == nullptr) {
        qWarning() << "PluginManagerPrivate::removeObject(): trying to remove null object";
        return;
    }

    if (!allObjects.contains(obj)) {
        qWarning() << "PluginManagerPrivate::removeObject(): object not in list:" << obj
                   << obj->objectName();
        return;
    }
    if (debugLeaks)
        qDebug() << "PluginManagerPrivate::removeObject" << obj << obj->objectName();

    emit q->aboutToRemoveObject(obj);
    QWriteLocker lock(&m_lock);
    allObjects.removeAll(obj);
}

/*!
    \internal
*/
void PluginManagerPrivate::loadPlugins()
{
    if (m_profilingVerbosity > 0)
        qDebug("Profiling started");

    const PluginSpecs queue = loadQueue();
    Utils::setMimeStartupPhase(MimeStartupPhase::PluginsLoading);
    {
        for (PluginSpec *spec : queue)
            loadPlugin(spec, PluginSpec::Loaded);
    }

    Utils::setMimeStartupPhase(MimeStartupPhase::PluginsInitializing);
    {
        for (PluginSpec *spec : queue)
            registerMimeFromPlugin(spec);
    }
    {
        for (PluginSpec *spec : queue)
            loadPlugin(spec, PluginSpec::Initialized);
    }

    {
        Utils::reverseForeach(queue, [this](PluginSpec *spec) {
            loadPlugin(spec, PluginSpec::Running);
            if (spec->state() == PluginSpec::Running) {
                delayedInitializeQueue.push(spec);
            } else {
                // Plugin initialization failed, so cleanup after it
                spec->kill();
            }
        });
    }
    emit q->pluginsChanged();

    delayedInitializeTimer.setInterval(DELAYED_INITIALIZE_INTERVAL);
    delayedInitializeTimer.setSingleShot(true);
    connect(&delayedInitializeTimer,
            &QTimer::timeout,
            this,
            &PluginManagerPrivate::startDelayedInitialize);
    delayedInitializeTimer.start();
}

void PluginManagerPrivate::loadPluginsAtRuntime(const QSet<PluginSpec *> &plugins)
{
    const bool allSoftloadable = allOf(plugins, &PluginSpec::isEffectivelySoftloadable);
    if (!allSoftloadable) {
        const QStringList notSoftLoadablePlugins = Utils::transform<QStringList>(
            Utils::filtered(plugins, std::not_fn(&PluginSpec::isEffectivelySoftloadable)),
            &PluginSpec::displayName);
        qWarning().noquote()
            << "PluginManagerPrivate::loadPluginsAtRuntime(): trying to load non-softloadable"
            << "plugin(s):" << notSoftLoadablePlugins.join(", ");
    }

    // load the plugins and their dependencies (if possible) ordered by dependency
    const QList<PluginSpec *> queue = filtered(loadQueue(), [&plugins](PluginSpec *spec) {
        // Is the current plugin already running, or not soft loadable?
        if (spec->state() == PluginSpec::State::Running || !spec->isEffectivelySoftloadable())
            return false;

        // Is the current plugin in the list of plugins to load?
        if (plugins.contains(spec))
            return true;

        // Is the current plugin a dependency of any of the plugins we want to load?
        return plugins.contains(spec) || Utils::anyOf(plugins, [spec](PluginSpec *other) {
                   return other->requiresAny({spec});
               });
    });

    std::queue<PluginSpec *> localDelayedInitializeQueue;
    for (PluginSpec *spec : queue)
        loadPlugin(spec, PluginSpec::Loaded);
    for (PluginSpec *spec : queue)
        registerMimeFromPlugin(spec);
    for (PluginSpec *spec : queue)
        loadPlugin(spec, PluginSpec::Initialized);
    Utils::reverseForeach(queue,
                          [this](PluginSpec *spec) { loadPlugin(spec, PluginSpec::Running); });
    Utils::reverseForeach(queue, [](PluginSpec *spec) {
        if (spec->state() == PluginSpec::Running) {
            const bool delay = spec->delayedInitialize();
            if (delay)
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        } else {
            // Plugin initialization failed, so cleanup after it
            spec->kill();
        }
    });
    emit q->pluginsChanged();
}

/*!
    \internal
*/
void PluginManagerPrivate::shutdown()
{
    stopAll();
    if (!asynchronousPlugins.isEmpty()) {
        shutdownEventLoop = new QEventLoop;
        shutdownEventLoop->exec();
    }
    deleteAll();
#ifdef EXTENSIONSYSTEM_WITH_TESTOPTION
    if (PluginManager::isScenarioRunning("TestModelManagerInterface")) {
        qDebug() << "Point 2: Expect the next call to Point 3 triggers a crash";
        QThread::sleep(5);
    }
#endif
    if (!allObjects.isEmpty()) {
        qDebug() << "There are" << allObjects.size() << "objects left in the plugin manager pool.";
        // Intentionally split debug info here, since in case the list contains
        // already deleted object we get at least the info about the number of objects;
        qDebug() << "The following objects left in the plugin manager pool:" << allObjects;
    }
}

/*!
    \internal
*/
const PluginSpecs PluginManagerPrivate::loadQueue()
{
    PluginSpecs queue;
    for (PluginSpec *spec : std::as_const(pluginSpecs)) {
        PluginSpecs circularityCheckQueue;
        loadQueue(spec, queue, circularityCheckQueue);
    }
    return queue;
}

/*!
    \internal
*/
bool PluginManagerPrivate::loadQueue(PluginSpec *spec,
                                     PluginSpecs &queue,
                                     PluginSpecs &circularityCheckQueue)
{
    if (queue.contains(spec))
        return true;
    // check for circular dependencies
    if (circularityCheckQueue.contains(spec)) {
        QString errorString = Tr::tr("Circular dependency detected:");
        errorString += QLatin1Char('\n');
        int index = circularityCheckQueue.indexOf(spec);
        for (int i = index; i < circularityCheckQueue.size(); ++i) {
            const PluginSpec *depSpec = circularityCheckQueue.at(i);
            errorString.append(
                Tr::tr("%1 (%2) depends on").arg(depSpec->name(), depSpec->version()));
            errorString += QLatin1Char('\n');
        }
        errorString.append(Tr::tr("%1 (%2)").arg(spec->name(), spec->version()));
        spec->setError(errorString);
        return false;
    }
    circularityCheckQueue.append(spec);
    // check if we have the dependencies
    if (spec->state() == PluginSpec::Invalid || spec->state() == PluginSpec::Read) {
        queue.append(spec);
        return false;
    }

    // add dependencies
    const QHash<PluginDependency, PluginSpec *> deps = spec->dependencySpecs();
    for (auto it = deps.cbegin(), end = deps.cend(); it != end; ++it) {
        // Skip test dependencies since they are not real dependencies but just force-loaded
        // plugins when running tests
        if (it.key().type == PluginDependency::Test)
            continue;
        PluginSpec *depSpec = it.value();
        if (!loadQueue(depSpec, queue, circularityCheckQueue)) {
            spec->setError(
                Tr::tr("Cannot load plugin because dependency failed to load: %1 (%2)\nReason: %3")
                    .arg(depSpec->name(), depSpec->version(), depSpec->errorString()));
            return false;
        }
    }
    // add self
    queue.append(spec);
    return true;
}

class LockFile
{
public:
    static QString filePath()
    {
        return QFileInfo(userSettings().fileName()).absolutePath() + '/'
               + QCoreApplication::applicationName() + '.'
               + QCryptographicHash::hash(QCoreApplication::applicationDirPath().toUtf8(),
                                          QCryptographicHash::Sha1)
                     .left(8)
                     .toHex()
               + ".lock";
    }

    static std::optional<QString> lockedPluginId()
    {
        const QString lockFilePath = LockFile::filePath();
        if (QFileInfo::exists(lockFilePath)) {
            QFile f(lockFilePath);
            if (f.open(QIODevice::ReadOnly)) {
                const auto pluginId = QString::fromUtf8(f.readLine()).trimmed();
                f.close();
                return pluginId;
            } else {
                qCDebug(pluginLog) << "Lock file" << lockFilePath << "exists but is not readable";
            }
        }
        return {};
    }

    LockFile(PluginSpec *spec)
        : m_filePath(filePath())
    {
        QDir().mkpath(QFileInfo(m_filePath).absolutePath());
        QFile f(m_filePath);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(spec->id().toUtf8());
            f.write("\n");
            f.close();
        } else {
            qCDebug(pluginLog) << "Cannot write lock file" << m_filePath;
        }
    }

    ~LockFile() { QFile::remove(m_filePath); }

private:
    QString m_filePath;
};

void PluginManagerPrivate::checkForProblematicPlugins()
{
    if (!enableCrashCheck)
        return;
    const std::optional<QString> pluginId = LockFile::lockedPluginId();
    if (pluginId) {
        PluginSpec *spec = pluginById(*pluginId);
        if (spec && !spec->isRequired()) {
            const QSet<PluginSpec *> dependents = PluginManager::pluginsRequiringPlugin(spec);
            auto dependentsNames = Utils::transform<QStringList>(dependents, &PluginSpec::name);
            std::sort(dependentsNames.begin(), dependentsNames.end());
            const QString dependentsList = dependentsNames.join(", ");
            const QString pluginsMenu = HostOsInfo::isMacHost()
                                            ? Tr::tr("%1 > About Plugins")
                                                  .arg(QGuiApplication::applicationDisplayName())
                                            : Tr::tr("Help > About Plugins");
            const QString otherPluginsText
                = Tr::tr("If you temporarily disable %1, the following plugins that depend on "
                         "it are also disabled: %2.")
                      .arg(spec->name(), dependentsList)
                  + "\n\n";
            const QString detailsText = (dependents.isEmpty() ? QString() : otherPluginsText)
                                        + Tr::tr("Disable plugins permanently in %1.")
                                              .arg(pluginsMenu);
            const QString text
                = Tr::tr("The last time you started %1, it seems to have closed because "
                         "of a problem with the \"%2\" "
                         "plugin. Temporarily disable the plugin?")
                      .arg(QGuiApplication::applicationDisplayName(), spec->name());
            QMessageBox dialog;
            dialog.setIcon(QMessageBox::Question);
            dialog.setText(text);
            dialog.setDetailedText(detailsText);
            QPushButton *disableButton = dialog.addButton(Tr::tr("Disable Plugin"),
                                                          QMessageBox::AcceptRole);
            dialog.addButton(Tr::tr("Continue"), QMessageBox::RejectRole);
            dialog.exec();
            if (dialog.clickedButton() == disableButton) {
                spec->setForceDisabled(true);
                for (PluginSpec *other : dependents)
                    other->setForceDisabled(true);
                enableDependenciesIndirectly();
            }
        }
    }
}

void PluginManager::checkForProblematicPlugins()
{
    d->checkForProblematicPlugins();
}

/*!
    Returns the PluginSpec corresponding to \a plugin.
*/

PluginSpec *PluginManager::specForPlugin(IPlugin *plugin)
{
    return findOrDefault(d->pluginSpecs, equal(&PluginSpec::plugin, plugin));
}

PluginSpec *PluginManager::specById(const QString &id)
{
    return d->pluginById(id);
}

bool PluginManager::specExists(const QString &id)
{
    return Utils::anyOf(d->pluginSpecs, Utils::equal(&PluginSpec::id, id));
}

bool PluginManager::specExistsAndIsEnabled(const QString &id)
{
    PluginSpec *spec = d->pluginById(id);
    return spec && spec->isEffectivelyEnabled();
}

static QString pluginListString(const QSet<PluginSpec *> &plugins)
{
    QStringList names = Utils::transform<QList>(plugins, &PluginSpec::name);
    names.sort();
    return names.join(QLatin1Char('\n'));
}

/*!
    Collects the dependencies of the \a plugins and asks the user if the
    corresponding plugins should be enabled or disabled (dependening on
    \a enable and using \a dialogParent as the parent for the dialog).

    Returns a (possibly) empty set of additional plugins that should be enabled or disabled
    respectively. Returns \c{std::nullopt} if the user canceled.
 */
std::optional<QSet<PluginSpec *>> PluginManager::askForEnablingPlugins(
    QWidget *dialogParent, const QSet<PluginSpec *> &plugins, bool enable)
{
    QSet<PluginSpec *> additionalPlugins;
    if (enable) {
        for (PluginSpec *spec : plugins) {
            for (PluginSpec *other : PluginManager::pluginsToEnableForPlugin(spec)) {
                if (!other->isEnabledBySettings())
                    additionalPlugins.insert(other);
            }
        }
        additionalPlugins.subtract(plugins);
        if (!additionalPlugins.isEmpty()) {
            if (QMessageBox::question(
                    dialogParent,
                    Tr::tr("Enabling Plugins"),
                    Tr::tr("Enabling\n%1\nwill also enable the following plugins:\n\n%2")
                        .arg(pluginListString(plugins), pluginListString(additionalPlugins)),
                    QMessageBox::Ok | QMessageBox::Cancel,
                    QMessageBox::Ok)
                != QMessageBox::Ok) {
                return {};
            }
        }
    } else {
        for (PluginSpec *spec : plugins) {
            for (PluginSpec *other : PluginManager::pluginsRequiringPlugin(spec)) {
                if (other->isEnabledBySettings())
                    additionalPlugins.insert(other);
            }
        }
        additionalPlugins.subtract(plugins);
        if (!additionalPlugins.isEmpty()) {
            if (QMessageBox::question(
                    dialogParent,
                    Tr::tr("Disabling Plugins"),
                    Tr::tr("Disabling\n%1\nwill also disable the following plugins:\n\n%2")
                        .arg(pluginListString(plugins), pluginListString(additionalPlugins)),
                    QMessageBox::Ok | QMessageBox::Cancel,
                    QMessageBox::Ok)
                != QMessageBox::Ok) {
                return {};
            }
        }
    }
    return additionalPlugins;
}

bool PluginManagerPrivate::acceptTermsAndConditions(PluginSpec *spec)
{
    if (pluginsWithAcceptedTermsAndConditions.contains(spec->id()))
        return true;

    if (!acceptTermsAndConditionsCallback) {
        spec->setError(Tr::tr("No callback set to accept terms and conditions"));
        return false;
    }

    if (!acceptTermsAndConditionsCallback(spec)) {
        spec->setError(Tr::tr("You did not accept the terms and conditions"));
        return false;
    }

    pluginsWithAcceptedTermsAndConditions.append(spec->id());
    userSettings().setValue(C_TANDCACCEPTED_PLUGINS, pluginsWithAcceptedTermsAndConditions);

    return true;
}

void PluginManagerPrivate::setAcceptTermsAndConditionsCallback(
    const std::function<bool(PluginSpec *)> &callback)
{
    acceptTermsAndConditionsCallback = callback;
}

/*!
    \internal
*/
void PluginManagerPrivate::loadPlugin(PluginSpec *spec, PluginSpec::State destState)
{
    if (spec->hasError() || spec->state() != destState - 1)
        return;

    // don't load disabled plugins.
    if (!spec->isEffectivelyEnabled() && destState == PluginSpec::Loaded)
        return;

    if (spec->termsAndConditions()) {
        if (!acceptTermsAndConditions(spec)) {
            spec->setError(Tr::tr("You did not accept the terms and conditions"));
            return;
        }
    }

    std::unique_ptr<LockFile> lockFile;
    if (enableCrashCheck && destState < PluginSpec::Stopped)
        lockFile.reset(new LockFile(spec));

    const std::string specId = spec->id().toStdString();

    switch (destState) {
    case PluginSpec::Running: {
        profilingReport(">initializeExtensions", spec);
        spec->initializeExtensions();
        profilingReport("<initializeExtensions",
                        spec,
                        &spec->performanceData().extensionsInitialized);
        return;
    }
    case PluginSpec::Deleted:
        profilingReport(">delete", spec);
        spec->kill();
        profilingReport("<delete", spec);
        return;
    default: break;
    }
    // check if dependencies have loaded without error
    if (!spec->isSoftLoadable()) {
        const QHash<PluginDependency, PluginSpec *> deps = spec->dependencySpecs();
        for (auto it = deps.cbegin(), end = deps.cend(); it != end; ++it) {
            if (it.key().type != PluginDependency::Required)
                continue;
            PluginSpec *depSpec = it.value();
            if (depSpec->state() != destState) {
                spec->setError(
                    Tr::tr(
                        "Cannot load plugin because dependency failed to load: %1(%2)\nReason: %3")
                        .arg(depSpec->name(), depSpec->version(), depSpec->errorString()));
                return;
            }
        }
    }
    switch (destState) {
    case PluginSpec::Loaded: {
        profilingReport(">loadLibrary", spec);
        spec->loadLibrary();
        profilingReport("<loadLibrary", spec, &spec->performanceData().load);
        break;
    }
    case PluginSpec::Initialized: {
        profilingReport(">initializePlugin", spec);
        spec->initializePlugin();
        profilingReport("<initializePlugin", spec, &spec->performanceData().initialize);
        break;
    }
    case PluginSpec::Stopped:
        profilingReport(">stop", spec);
        if (spec->stop() == IPlugin::AsynchronousShutdown) {
            asynchronousPlugins << spec;
            connect(spec->plugin(), &IPlugin::asynchronousShutdownFinished, this, [this, spec] {
                asynchronousPlugins.remove(spec);
                if (asynchronousPlugins.isEmpty())
                    shutdownEventLoop->exit();
            });
        }
        profilingReport("<stop", spec);
        break;
    default: break;
    }
}

/*!
    \internal
*/
void PluginManagerPrivate::setPluginPaths(const FilePaths &paths)
{
    qCDebug(pluginLog) << "Plugin search paths:" << paths;
    qCDebug(pluginLog) << "Required IID:" << pluginIID;
    pluginPaths = paths;
    readSettings();
    readPluginPaths();
}

static const FilePaths pluginFiles(const FilePaths &pluginPaths)
{
    FilePaths pluginFiles;
    FilePaths searchPaths = pluginPaths;
    while (!searchPaths.isEmpty()) {
        const FilePath dir = searchPaths.takeFirst().absoluteFilePath();
        const FilePaths files = dir.dirEntries(QDir::Files | QDir::NoSymLinks);
        pluginFiles += Utils::filtered(files, [](const FilePath &path) {
            return QLibrary::isLibrary(path.toFSPathString());
        });
        const FilePaths dirs = dir.dirEntries(QDir::Dirs | QDir::NoDotAndDotDot);
        searchPaths += dirs;
    }
    return pluginFiles;
}

void PluginManagerPrivate::addPlugins(const PluginSpecs &specs)
{
    pluginSpecs += specs;

    for (PluginSpec *spec : specs) {
        // defaultDisabledPlugins and defaultEnabledPlugins from install settings
        // is used to override the defaults read from the plugin spec
        if (spec->isEnabledByDefault() && defaultDisabledPlugins.contains(spec->id())) {
            spec->setEnabledByDefault(false);
            spec->setEnabledBySettings(false);
        } else if (!spec->isEnabledByDefault() && defaultEnabledPlugins.contains(spec->id())) {
            spec->setEnabledByDefault(true);
            spec->setEnabledBySettings(true);
        }
        if (!spec->isEnabledByDefault() && forceEnabledPlugins.contains(spec->id()))
            spec->setEnabledBySettings(true);
        if (spec->isEnabledByDefault() && disabledPlugins.contains(spec->id()))
            spec->setEnabledBySettings(false);

        pluginCategories[spec->category()].append(spec);
    }
    resolveDependencies();
    enableDependenciesIndirectly();
    checkForDuplicatePlugins();
    // ensure deterministic plugin load order by sorting
    Utils::sort(pluginSpecs, &PluginSpec::id);
    emit q->pluginsChanged();
}

static const char PLUGINS_TO_INSTALL_KEY[] = "PluginsToInstall";
static const char PLUGINS_TO_REMOVE_KEY[] = "PluginsToRemove";

Result<> PluginManagerPrivate::removePluginOnRestart(const QString &pluginId)
{
    const PluginSpec *pluginSpec = pluginById(pluginId);

    if (!pluginSpec)
        return ResultError(Tr::tr("Plugin not found."));

    const Result<FilePaths> filePaths = pluginSpec->filesToUninstall();
    if (!filePaths)
        return ResultError(filePaths.error());

    const QVariantList list = Utils::transform(*filePaths, &FilePath::toVariant);

    QtcSettings &settings = Utils::userSettings();
    settings.setValue(PLUGINS_TO_REMOVE_KEY, settings.value(PLUGINS_TO_REMOVE_KEY).toList() + list);

    settings.sync();
    return ResultOk;
}

static QList<QPair<FilePath, FilePath>> readPluginInstallList()
{
    QtcSettings &settings = Utils::userSettings();
    int size = settings.beginReadArray(PLUGINS_TO_INSTALL_KEY);

    QList<QPair<FilePath, FilePath>> installList;
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        installList.append({FilePath::fromVariant(settings.value("src")),
                            FilePath::fromVariant(settings.value("dest"))});
    }
    settings.endArray();
    return installList;
}

void PluginManagerPrivate::installPluginOnRestart(const FilePath &src, const FilePath &dest)
{
    QtcSettings &settings = Utils::userSettings();
    const QList<QPair<FilePath, FilePath>> list = readPluginInstallList() << qMakePair(src, dest);

    settings.beginWriteArray(PLUGINS_TO_INSTALL_KEY);
    for (int i = 0; i < list.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("src", list.at(i).first.toVariant());
        settings.setValue("dest", list.at(i).second.toVariant());
    }
    settings.endArray();

    settings.sync();
}

void PluginManagerPrivate::removePluginsAfterRestart()
{
    QtcSettings &settings = Utils::userSettings();
    const FilePaths removeList = Utils::transform(settings.value(PLUGINS_TO_REMOVE_KEY).toList(),
                                                  &FilePath::fromVariant);

    for (const FilePath &path : removeList) {
        Result<> r = ResultError(Tr::tr("It does not exist."));
        if (path.isFile())
            r = path.removeFile();
        else if (path.isDir())
            r = path.removeRecursively();

        if (!r)
            qCWarning(pluginLog()) << "Failed to remove" << path << ":" << r.error();
    }

    settings.remove(PLUGINS_TO_REMOVE_KEY);
}

void PluginManagerPrivate::installPluginsAfterRestart()
{
    QTC_CHECK(pluginSpecs.isEmpty());

    const QList<QPair<FilePath, FilePath>> installList = readPluginInstallList();

    for (const auto &[src, dest] : installList) {
        if (!src.exists()) {
            qCWarning(pluginLog()) << "Cannot install source " << src << ", it does not exist";
            continue;
        }

        if (dest.isDir()) {
            if (auto result = dest.removeRecursively(); !result) {
                qCWarning(pluginLog()) << "Failed to remove" << dest << ":" << result.error();
                continue;
            }
        } else if (dest.isFile()) {
            if (const Result<> result = dest.removeFile(); !result) {
                qCWarning(pluginLog()) << "Failed to remove" << dest << ":" << result.error();
                continue;
            }
        }

        if (src.isFile()) {
            if (!dest.createDir()) {
                qCWarning(pluginLog()) << "Cannot install file" << src << "to" << dest
                                       << "because the destination directory cannot be created";
                continue;
            }
        }

        Utils::Result<> result = src.isDir() ? src.copyRecursively(dest)
                                             : src.copyFile(dest / src.fileName());

        if (!result) {
            qCWarning(pluginLog())
                << "Failed to install" << src << "to" << dest << ":" << result.error();
            continue;
        }

        result = src.isDir() ? src.removeRecursively() : src.removeFile();
        if (!result)
            qCWarning(pluginLog())
                << "Failed to remove the source file in" << src << ":" << result.error();
    }

    Utils::userSettings().remove(PLUGINS_TO_INSTALL_KEY);
}

/*!
    \internal
*/
void PluginManagerPrivate::readPluginPaths()
{
    PluginSpecs newSpecs;

    // from the file system
    for (const FilePath &pluginFile : pluginFiles(pluginPaths)) {
        Result<std::unique_ptr<PluginSpec>> spec = readCppPluginSpec(pluginFile);
        if (!spec) {
            qCInfo(pluginLog).noquote() << QString("Ignoring plugin \"%1\" because: %2")
                                               .arg(pluginFile.toUserOutput())
                                               .arg(spec.error());
            continue;
        }
        newSpecs.append(spec->release());
    }

    // static
    for (const QStaticPlugin &plugin : QPluginLoader::staticPlugins()) {
        Result<std::unique_ptr<PluginSpec>> spec = readCppPluginSpec(plugin);
        if (!spec) {
            qCInfo(pluginLog).noquote()
                << QString("Ignoring static plugin because: %2").arg(spec.error());
            continue;
        }
        newSpecs.append(spec->release());
    }

    newSpecs = Utils::filtered(newSpecs, [this](PluginSpec *spec) {
        return pluginById(spec->id()) == nullptr;
    });

    if (newSpecs.empty())
        return;

    addPlugins(newSpecs);
}

void PluginManagerPrivate::resolveDependencies()
{
    for (PluginSpec *spec : std::as_const(pluginSpecs))
        spec->resolveDependencies(pluginSpecs);
}

void PluginManagerPrivate::enableDependenciesIndirectly()
{
    for (PluginSpec *spec : std::as_const(pluginSpecs))
        spec->setEnabledIndirectly(false);
    // cannot use reverse loadQueue here, because test dependencies can introduce circles
    PluginSpecs queue = Utils::filtered(pluginSpecs, &PluginSpec::isEffectivelyEnabled);
    while (!queue.isEmpty()) {
        PluginSpec *spec = queue.takeFirst();
        queue += spec->enableDependenciesIndirectly(containsTestSpec(spec));
    }
}

// Look in argument descriptions of the specs for the option.
PluginSpec *PluginManagerPrivate::pluginForOption(const QString &option,
                                                  bool *requiresArgument) const
{
    // Look in the plugins for an option
    *requiresArgument = false;
    for (PluginSpec *spec : std::as_const(pluginSpecs)) {
        PluginArgumentDescription match = Utils::findOrDefault(spec->argumentDescriptions(),
                                                               [option](
                                                                   PluginArgumentDescription pad) {
                                                                   return pad.name == option;
                                                               });
        if (!match.name.isEmpty()) {
            *requiresArgument = !match.parameter.isEmpty();
            return spec;
        }
    }
    return nullptr;
}

PluginSpec *PluginManagerPrivate::pluginById(const QString &id_in) const
{
    QString id = id_in;
    // Plugin ids are always lower case. So the id argument should be too.
    QTC_ASSERT(id.isLower(), id = id.toLower());
    return Utils::findOrDefault(pluginSpecs, Utils::equal(&PluginSpec::id, id));
}

void PluginManagerPrivate::increaseProfilingVerbosity()
{
    m_profilingVerbosity++;
    if (!m_profileTimer)
        PluginManager::startProfiling();
}

void PluginManagerPrivate::enableTracing(const QString &filePath)
{
    const QString jsonFilePath = filePath.endsWith(".json") ? filePath : filePath + ".json";
}

void PluginManagerPrivate::profilingReport(const char *what, const PluginSpec *spec, qint64 *target)
{
    if (m_profileTimer) {
        const qint64 absoluteElapsedMS = m_profileTimer->elapsed();
        const qint64 elapsedMS = absoluteElapsedMS - m_profileElapsedMS;
        m_profileElapsedMS = absoluteElapsedMS;
        if (m_profilingVerbosity > 0) {
            qDebug("%-22s %-40s %8lldms (%8lldms)",
                   what,
                   qPrintable(spec->id()),
                   absoluteElapsedMS,
                   elapsedMS);
        }
        if (target) {
            QString tc;
            *target = elapsedMS;
            tc = spec->id() + '_';
            tc += QString::fromUtf8(QByteArray(what + 1));
            Utils::Benchmarker::report("loadPlugins", tc, elapsedMS);
        }
    }
}

QString PluginManagerPrivate::profilingSummary(qint64 *totalOut) const
{
    QString summary;
    const PluginSpecs specs = Utils::sorted(pluginSpecs, [](PluginSpec *s1, PluginSpec *s2) {
        return s1->performanceData().total() < s2->performanceData().total();
    });
    const qint64 total = std::accumulate(specs.constBegin(),
                                         specs.constEnd(),
                                         0,
                                         [](qint64 t, PluginSpec *s) {
                                             return t + s->performanceData().total();
                                         });
    for (PluginSpec *s : specs) {
        if (!s->isEffectivelyEnabled())
            continue;
        const qint64 t = s->performanceData().total();
        summary += QString("%1 %2ms   ( %3% ) (%4)\n")
                       .arg(s->id(), -34)
                       .arg(t, 8)
                       .arg(100.0 * t / total, 5, 'f', 2)
                       .arg(s->performanceData().summary());
    }
    summary += QString("Total plugins: %1ms\n").arg(total, 8);
    summary += QString("Total startup: %1ms\n").arg(m_totalStartupMS, 8);
    if (totalOut)
        *totalOut = total;
    return summary;
}

void PluginManagerPrivate::printProfilingSummary() const
{
    if (m_profilingVerbosity > 0) {
        qint64 total;
        const QString summary = profilingSummary(&total);
        qDebug() << qPrintable(summary);
        Utils::Benchmarker::report("loadPlugins", "Total", total);
    }
}

static inline QString getPlatformName()
{
    if (HostOsInfo::isMacHost())
        return QLatin1String("OS X");
    else if (HostOsInfo::isAnyUnixHost())
        return QLatin1String(HostOsInfo::isLinuxHost() ? "Linux" : "Unix");
    else if (HostOsInfo::isWindowsHost())
        return QLatin1String("Windows");
    return QLatin1String("Unknown");
}

QString PluginManager::platformName()
{
    static const QString result = getPlatformName() + " (" + QSysInfo::prettyProductName() + ')';
    return result;
}

bool PluginManager::isInitializationDone()
{
    return d->m_isInitializationDone;
}

bool PluginManager::isShuttingDown()
{
    return !d || d->m_isShuttingDown;
}

/*!
    Retrieves one object with \a name from the object pool.
    \sa addObject()
*/

QObject *PluginManager::getObjectByName(const QString &name)
{
    QReadLocker lock(&d->m_lock);
    return Utils::findOrDefault(allObjects(),
                                [&name](const QObject *obj) { return obj->objectName() == name; });
}

void PluginManager::startProfiling()
{
    d->m_profileTimer.reset(new QElapsedTimer);
    d->m_profileTimer->start();
    d->m_profileElapsedMS = 0;
}

void PluginManager::setAcceptTermsAndConditionsCallback(
    const std::function<bool(PluginSpec *)> &callback)
{
    d->setAcceptTermsAndConditionsCallback(callback);
}

void PluginManager::setTermsAndConditionsAccepted(PluginSpec *spec)
{
    if (spec->termsAndConditions()
        && !d->pluginsWithAcceptedTermsAndConditions.contains(spec->id())) {
        d->pluginsWithAcceptedTermsAndConditions.append(spec->id());
        Utils::userSettings().setValue(C_TANDCACCEPTED_PLUGINS,
                                       d->pluginsWithAcceptedTermsAndConditions);
    }
}

} // namespace ExtensionSystem
