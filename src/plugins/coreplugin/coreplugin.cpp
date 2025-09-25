#include "coreplugin.hpp"
#include "coreplugintr.h"
#include "icore.h"
#include "mainwindow.h"
#include "themechooser.h"

#include <extensionsystem/iplugin.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/guiutils.h>
#include <utils/id.h>
#include <utils/theme/theme_p.h>
#include <utils/utils.hpp>

#include <QApplication>
#include <QPointer>

namespace Plugin {

static QPointer<MainWindow> mainwindowPtr;

QWidget *dialogParent()
{
    QWidget *active = QApplication::activeModalWidget();
    if (!active)
        active = QApplication::activeWindow();
    if (!active || active->windowFlags().testFlag(Qt::SplashScreen)
        || active->windowFlags().testFlag(Qt::Popup)) {
        active = mainwindowPtr;
    }
    return active;
}

class CorePlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "coreplugin.json")

    struct CoreArguments
    {
        QColor overrideColor;
        Utils::Id themeId;
        bool presentationMode = false;
    };

    CoreArguments parseArguments(const QStringList &arguments)
    {
        CoreArguments args;
        for (int i = 0; i < arguments.size(); ++i) {
            if (arguments.at(i) == QLatin1String("-color")) {
                const QString colorcode(arguments.at(i + 1));
                args.overrideColor = QColor(colorcode);
                i++; // skip the argument
            }
            if (arguments.at(i) == QLatin1String("-presentationMode"))
                args.presentationMode = true;
            if (arguments.at(i) == QLatin1String("-theme")) {
                args.themeId = Utils::Id::fromString(arguments.at(i + 1));
                i++; // skip the argument
            }
        }
        return args;
    }

public:
    ~CorePlugin() override {}

    Utils::Result<> initialize(const QStringList &arguments) override
    {
        if (ThemeEntry::availableThemes().isEmpty()) {
            return Utils::ResultError(Tr::tr("No themes found in installation."));
        }

        const auto args = parseArguments(arguments);
        auto *themeFromArg = ThemeEntry::createTheme(args.themeId);
        auto *theme = themeFromArg ? themeFromArg
                                   : ThemeEntry::createTheme(ThemeEntry::themeSetting());
        Utils::Theme::setInitialPalette(theme); // Initialize palette before setting it
        Utils::setCreatorTheme(theme);

        m_corePtr.reset(new ICore);
        m_mainWindowPtr.reset(new MainWindow);
        mainwindowPtr = m_mainWindowPtr.data();
        Utils::setDialogParentGetter(dialogParent);

        return Utils::ResultOk;
    }

    void extensionsInitialized() override { m_mainWindowPtr->extensionsInitialized(); }

    QObject *remoteCommand(const QStringList &, const QString &, const QStringList &) override
    {
        Utils::restoreAndActivate(mainwindowPtr.data());
        return nullptr;
    }

private:
    QScopedPointer<MainWindow> m_mainWindowPtr;
    QScopedPointer<ICore> m_corePtr;
};

} // namespace Plugin

#include "coreplugin.moc"
