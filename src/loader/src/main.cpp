#include <utility>
#include <optional>

#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSettings>
#include <QtGui/QScreen>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>

#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>

#include <SingleApplication>

#include <CoreApi/applicationinfo.h>
#include <CoreApi/logger.h>
#include <CoreApi/runtimeinterface.h>
#include <CoreApi/private/runtimeinterface_p.h>

#include "loaderspec.h"
#include "splashscreen.h"

using Loader::LoaderSpec;
using Loader::SplashScreen;

using namespace ExtensionSystem;
using namespace Core;

// Use native messageBox to display message, instead of QMessageBox
#define CKLOADER_USE_NATIVE_MESSAGEBOX

static Q_LOGGING_CATEGORY(ckLoader, "ck.loader");

static const SingleApplication::Options opts = SingleApplication::User |              //
                                               SingleApplication::ExcludeAppPath |    //
                                               SingleApplication::ExcludeAppVersion | //
                                               SingleApplication::SecondaryNotification;

enum {
    OptionIndent = 4,
    DescriptionIndent = 34,
};

static const char fixedOptionsC[] = " [options]... [files]...\n";

// Default arguments
static const char HELP_OPTION1[] = "-h";
static const char HELP_OPTION2[] = "--help";
static const char VERSION_OPTION1[] = "-v";
static const char VERSION_OPTION2[] = "--version";
static const char PLUGIN_PATH_OPTION[] = "--plugin-path";

// Optional arguments
static const char ALLOW_ROOT_OPTION[] = "--allow-root";

// Global variables
static QSplashScreen *g_splash = nullptr;
static LoaderSpec *g_loadSpec = nullptr;

// Format as <pre> HTML
static inline QString toHtml(const QString &t) {
    QString res = t;
    res.replace(QLatin1Char('&'), QLatin1String("&amp;"));
    res.replace(QLatin1Char('<'), QLatin1String("&lt;"));
    res.replace(QLatin1Char('>'), QLatin1String("&gt;"));
    res.prepend(QLatin1String("<html><pre>"));
    res.append(QLatin1String("</pre></html>"));
    return res;
}

static inline void indent(QTextStream &str, int indent) {
    const QChar blank = QLatin1Char(' ');
    for (int i = 0; i < indent; i++)
        str << blank;
}

static inline void formatOption(QTextStream &str, const QStringList &opts, const QString &parm,
                                const QString &description) {
    QString opt = opts.join('/');
    int remainingIndent = DescriptionIndent - OptionIndent - opt.size();
    indent(str, OptionIndent);
    str << opt;
    if (!parm.isEmpty()) {
        str << " <" << parm << '>';
        remainingIndent -= 3 + parm.size();
    }
    if (remainingIndent >= 1) {
        indent(str, remainingIndent);
    } else {
        str << '\n';
        indent(str, DescriptionIndent);
    }
    str << description << '\n';
}

void displayError(const QString &t, int exitCode = -1) {
#ifndef CKLOADER_USE_NATIVE_MESSAGEBOX
    QMessageBox msgbox;
    msgbox.setIcon(QMessageBox::Critical);
    msgbox.setWindowTitle(qApp->applicationDisplayName());
    msgbox.setText(toHtml(t));
    msgbox.show();
    if (g_splash) {
        g_splash->finish(&msgbox);
    }
    std::ignore = msgbox.exec();
#else
    if (g_splash) {
        g_splash->close();
    }
    ApplicationInfo::messageBox(nullptr, ApplicationInfo::Critical, qApp->applicationDisplayName(), t);
#endif
    std::exit(exitCode);
}

static inline void displayHelpText(const QString &t) {
#if 1
    QMessageBox msgbox;
    msgbox.setIcon(QMessageBox::Information);
    msgbox.setWindowTitle(qApp->applicationDisplayName());
    msgbox.setText(toHtml(t));
    msgbox.show();
    if (g_splash) {
        g_splash->finish(&msgbox);
    }
    std::ignore = msgbox.exec();
#else
    if (g_splash) {
        g_splash->close();
    }
    ApplicationInfo::messageBox(nullptr, ApplicationInfo::Information, qApp->applicationDisplayName(), t);
#endif

    std::exit(0);
}

static inline void printVersion(const PluginSpec *coreplugin) {
    QString version;
    QTextStream str(&version);
    str << '\n'
        << qApp->applicationName() << ' ' << coreplugin->version() << " based on Qt " << qVersion()
        << "\n\n";
    PluginManager::formatPluginVersions(str);
    str << '\n' << coreplugin->copyright() << '\n';
    displayHelpText(version);
}

static inline void printHelp() {
    QString help;
    QTextStream str(&help);

    str << "Usage: " << qApp->applicationName() << fixedOptionsC;

    formatOption(str, {HELP_OPTION1, HELP_OPTION2}, {}, "Display this help");
    formatOption(str, {VERSION_OPTION1, VERSION_OPTION2}, {}, "Display application version");

    for (const auto &item : std::as_const(g_loadSpec->extraArguments)) {
        formatOption(str, item.options, item.param, item.description);
    }
    if (!g_loadSpec->allowRoot) {
        formatOption(str, {ALLOW_ROOT_OPTION}, {}, "Allow running with root privilege");
    }

    formatOption(str, {PLUGIN_PATH_OPTION}, "path", "Add a custom search path for plugins");

    PluginManager::formatOptions(str, OptionIndent, DescriptionIndent);
    if (PluginManager::instance()) {
        PluginManager::PluginManager::formatPluginOptions(str, OptionIndent, DescriptionIndent);
    }
    displayHelpText(help);
}

static inline QString msgCoreLoadFailure(const QString &why) {
    return QCoreApplication::translate("Application", "Failed to load core: %1").arg(why);
}

namespace {

    class ArgumentParser {
    public:
        bool allowRoot;
        bool showHelp;
        QStringList customPluginPaths;

        ArgumentParser() : allowRoot(false), showHelp(false) {
        }

        void parse(QStringList &arguments) {
            QMutableStringListIterator it(arguments);
            while (it.hasNext()) {
                const QString &arg = it.next();
                if (!g_loadSpec->allowRoot && arg == QLatin1String(ALLOW_ROOT_OPTION)) {
                    it.remove();
                    allowRoot = true;
                } else if (arg == QLatin1String(PLUGIN_PATH_OPTION)) {
                    it.remove();
                    if (it.hasNext()) {
                        customPluginPaths << it.next();
                        it.remove();
                    }
                } else if (arg == HELP_OPTION1 || arg == HELP_OPTION2) {
                    showHelp = true;
                } else if (arg.startsWith('-')) {
                    if (it.hasNext()) {
                        it.next();
                    }
                }
            }
        }
    };

}

int __main__(LoaderSpec *loadSpec) {
    g_loadSpec = loadSpec;

    // Global instances must be created
    QApplication &a = *qApp;

    QStringList arguments = a.arguments();
    ArgumentParser argsParser;

    // Process command line arguments
    {
        argsParser.parse(arguments);

        // Root privilege detection
        if (!g_loadSpec->allowRoot && !argsParser.allowRoot && !argsParser.showHelp &&
            ApplicationInfo::isUserRoot()) {
            QString msg =
                QCoreApplication::translate(
                    "Application", "You are trying to start %1 as the %2, which is "
                                   "extremely dangerous and therefore strongly not recommended.\n\n"
                                   "You can supress this warning by starting the application with \"--allow-root\" option.")
                    .arg(qApp->applicationDisplayName(),
#ifdef Q_OS_WINDOWS
                         QCoreApplication::translate("Application", "Administrator")
#else
                         QCoreApplication::translate("Application", "Root")
#endif
                    );
            ApplicationInfo::messageBox(nullptr, ApplicationInfo::Warning, qApp->applicationDisplayName(),
                                        msg);
        }

        // If you need to show help, we simply ignore this error and continue loading plugins
        int code = -1;
        if (!loadSpec->preprocessArguments(arguments, &code) && !argsParser.showHelp) {
            return code;
        }
    }

    // QtCreator ExtensionSystem plugin manager
    PluginManager pluginManager;
    pluginManager.setPluginIID(loadSpec->pluginIID);
    pluginManager.setSettings(loadSpec->createExtensionSystemSettings(QSettings::UserScope));
    pluginManager.setGlobalSettings(
        loadSpec->createExtensionSystemSettings(QSettings::SystemScope));

    // ChorusKit plugin database
    RuntimeInterface runtimeInterface;
    runtimeInterface.setSettings(loadSpec->createChorusKitSettings(QSettings::UserScope));
    runtimeInterface.setGlobalSettings(loadSpec->createChorusKitSettings(QSettings::SystemScope));

    SplashScreen splash;
    g_splash = &splash;
    runtimeInterface.setSplash(&splash);
    loadSpec->splashWillShow(&splash);

    splash.applyConfig(loadSpec->splashConfigPath);
    splash.show();

    // QFontDatabase needs a lot of time to initialize, so we show splash first and then show texts
    splash.showTexts();

    loadSpec->splashShown(&splash);

    // Update loader text
    splash.showMessage(QCoreApplication::translate("Application", "Searching plugins..."));

    QStringList pluginPaths = loadSpec->pluginPaths + argsParser.customPluginPaths;
    pluginManager.setPluginPaths(pluginPaths);

    // Parse arguments again
    QMap<QString, QString> foundAppOptions;
    if (arguments.size() > 1) {
        QMap<QString, bool> appOptions;
        appOptions.insert(QLatin1String(HELP_OPTION1), false);
        appOptions.insert(QLatin1String(HELP_OPTION2), false);
        appOptions.insert(QLatin1String(VERSION_OPTION1), false);
        appOptions.insert(QLatin1String(VERSION_OPTION2), false);
        QString errorMessage;
        if (!PluginManager::parseOptions(arguments, appOptions, &foundAppOptions, &errorMessage)) {
            displayError(errorMessage);
            printHelp();
            return -1;
        }
    }

    // Load plugins
    const auto plugins = PluginManager::plugins();
    PluginSpec *coreplugin = nullptr;
    for (auto spec : std::as_const(plugins)) {
        if (spec->name() == loadSpec->coreName) {
            coreplugin = spec;
            break;
        }
    }

    // Check core plugin
    const auto &checkCorePlugin = [](PluginSpec *coreplugin, QString &reason) {
        if (!coreplugin) {
            reason = QCoreApplication::translate("Application", "Could not find Core plugin!");
            return false;
        }

        if (!coreplugin->isEffectivelyEnabled()) {
            reason = QCoreApplication::translate("Application", "Core plugin is disabled.");
            return false;
        }
        if (coreplugin->hasError()) {
            reason = coreplugin->errorString();
            return false;
        }
        return true;
    };

    if (QString reason; !checkCorePlugin(coreplugin, reason)) {
        // Ignore errors if need to show help
        if (argsParser.showHelp) {
            printHelp();
            return 0;
        }
        displayError(msgCoreLoadFailure(reason));
        return 1;
    }

    // Show version or full help information
    if (foundAppOptions.contains(QLatin1String(VERSION_OPTION1)) ||
        foundAppOptions.contains(VERSION_OPTION2)) {
        printVersion(coreplugin);
        return 0;
    }

    if (foundAppOptions.contains(QLatin1String(HELP_OPTION1)) ||
        foundAppOptions.contains(QLatin1String(HELP_OPTION2))) {
        printHelp();
        return 0;
    }

    std::optional<SingleApplication> singleHook;
    if (loadSpec->single) {
        // Initialize singleton handle
        singleHook.emplace(qApp, true, opts);
        if (auto &single = *singleHook; !single.isPrimary()) {
            qCDebug(ckLoader) << "primary instance already running. PID:" << single.primaryPid();

            // This eventually needs moved into the NotepadNextApplication to keep
            // sending/receiving logic in the same place
            QByteArray buffer;
            QDataStream stream(&buffer, QIODevice::WriteOnly);

            stream << PluginManager::serializedArguments();
            single.sendMessage(buffer);

            qCDebug(ckLoader) << "secondary instance closing...";

            return 0;
        } else {
            qCDebug(ckLoader) << "primary instance initializing...";
        }
    }

    Logger logger;
    RuntimeInterface::setLogger(&logger);
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        RuntimeInterface::logger()->log(Logger::fromQtMsgType(type), context.category, msg);
    });

    qInfo().noquote() << QApplication::applicationName() << QApplication::applicationVersion() << "starting";
    qInfo() << "Application started at" << RuntimeInterface::startTime().toString() << RuntimeInterface::startTime().toUTC().toString(Qt::ISODateWithMs);

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [&] {
        qInfo() << "Quitting";
    });

    loadSpec->beforeLoadPlugins();

    // Update loader text
    splash.showMessage(QCoreApplication::translate("Application", "Loading plugins..."));

    // Load all plugins
    PluginManager::loadPlugins();
    if (coreplugin->hasError()) {
        displayError(msgCoreLoadFailure(coreplugin->errorString()));
        return 1;
    }

    loadSpec->afterLoadPlugins();

    if (singleHook.has_value()) {
        // Set up remote arguments handler
        QObject::connect(&singleHook.value(), &SingleApplication::receivedMessage,
                         [&](quint32 instanceId, QByteArray message) {
                             QDataStream stream(&message, QIODevice::ReadOnly);
                             QString msg;
                             stream >> msg;
                             qCDebug(ckLoader).noquote().nospace()
                                 << " remote message from " << instanceId << ", " << msg;
                             pluginManager.remoteArguments(msg, nullptr);
                         });
    }

    // shutdown plugin manager on the exit
    QObject::connect(&a, &QApplication::aboutToQuit, &pluginManager, &PluginManager::shutdown);

    return RuntimeInterfacePrivate::restartOrExit(a.exec());
}
