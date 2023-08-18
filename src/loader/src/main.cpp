#include <QApplication>
#include <QDir>
#include <QLoggingCategory>
#include <QMainWindow>
#include <QProcess>
#include <QTextStream>

#include <QMAppExtension.h>
#include <QMConsole.h>
#include <QMCss.h>
#include <QMDecoratorV2.h>
#include <QMSystem.h>

#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>

#include <singleapplication.h>

#include "config/loadconfig.h"
#include "splash/SplashScreen.h"

#include "loaderconfig.h"

#define USE_NATIVE_MESSAGEBOX

Q_LOGGING_CATEGORY(loaderLog, "apploader")

using namespace ExtensionSystem;

static const SingleApplication::Options opts =
    SingleApplication::ExcludeAppPath | SingleApplication::ExcludeAppVersion | SingleApplication::SecondaryNotification;


enum { OptionIndent = 4, DescriptionIndent = 34 };

static const char fixedOptionsC[] = " [options]... [files]...\n";

static const char HELP_OPTION1[] = "-h";
static const char HELP_OPTION2[] = "--help";
static const char VERSION_OPTION1[] = "-v";
static const char VERSION_OPTION2[] = "--version";
static const char PLUGIN_PATH_OPTION[] = "--plugin-path";

static const char ALLOW_ROOT_OPTION[] = "--allow-root";

static QSplashScreen *g_splash = nullptr;

static LoaderConfiguration *g_loadConfig = nullptr;

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

static inline void displayError(const QString &t) {
#ifndef USE_NATIVE_MESSAGEBOX
    QMessageBox msgbox;
    msgbox.setIcon(QMessageBox::Critical);
    msgbox.setWindowTitle(qApp->applicationName());
    msgbox.setText(toHtml(t));
    msgbox.show();
    if (g_splash) {
        g_splash->finish(&msgbox);
    }
    msgbox.exec();
#else
    if (g_splash) {
        g_splash->close();
    }
    qmCon->MsgBox(nullptr, QMConsole::Critical, qApp->applicationName(), t);
#endif
    QMOs::exitApp(-1);
}

static inline void displayHelpText(const QString &t) {
#ifndef USE_NATIVE_MESSAGEBOX
    QMessageBox msgbox;
    msgbox.setIcon(QMessageBox::Information);
    msgbox.setWindowTitle(qApp->applicationName());
    msgbox.setText(toHtml(t));
    msgbox.show();
    if (g_splash) {
        g_splash->finish(&msgbox);
    }
    msgbox.exec();
#else
    if (g_splash) {
        g_splash->close();
    }
    qmCon->MsgBox(nullptr, QMConsole::Information, qApp->applicationName(), t);
#endif

    QMOs::exitApp(0);
}

static inline void printVersion(const PluginSpec *coreplugin) {
    QString version;
    QTextStream str(&version);
    str << '\n' << qApp->applicationName() << ' ' << coreplugin->version() << " based on Qt " << qVersion() << "\n\n";
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

    for (const auto &item : qAsConst(g_loadConfig->extraArguments)) {
        formatOption(str, item.options, item.param, item.description);
    }
    if (!g_loadConfig->allowRoot) {
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

class Restarter {
public:
    Restarter(const QString &workingDir) : m_workingDir(workingDir) {
    }

    int restartOrExit(int exitCode) {
        return qApp->property("restart").toBool() ? restart(exitCode) : exitCode;
    }

    int restart(int exitCode) {
        QProcess::startDetached(QApplication::applicationFilePath(), QApplication::arguments(), m_workingDir);
        return exitCode;
    }

private:
    QString m_workingDir;
};

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
            if (!g_loadConfig->allowRoot && arg == QLatin1String(ALLOW_ROOT_OPTION)) {
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

class ConfigFileLoader {
public:
    ConfigFileLoader() {
    }

    void load(const QString &fileName, SplashScreen *splash) {
        QString configDir = QMFs::PathFindDirPath(fileName);

        // Load configuration
        LoadConfig configFile;
        if (configFile.load(fileName)) {
            if (!configFile.splashImage.isEmpty()) {
                QString path = configFile.splashImage;
                if (QDir::isRelativePath(path)) {
                    path = configDir + "/" + path;
                }
                splashImagePath = path;
            }
            if (configFile.splashSize.size() == 2) {
                splashSize = QSize(configFile.splashSize.front(), configFile.splashSize.back());
            }
            for (const auto &file : qAsConst(configFile.resourceFiles)) {
                if (QDir::isRelativePath(file)) {
                    resourcesFiles.append(configDir + "/" + file);
                } else {
                    resourcesFiles.append(file);
                }
            }
        }

        splashImage = QImage(splashImagePath);
        // splashImage = QImage();
        if (splashImage.isNull()) {
            splashImagePath = ":/A60.jpg";
            splashImage = QImage(splashImagePath);
            splashSize = splashImage.size();
        }

        if (splashSize.isEmpty()) {
            splashSize = splashImage.size();
        }

        // Setup splash
        double ratio = splash->screen()->logicalDotsPerInch() / QMOs::unitDpi() * 0.8;
        if (configFile.resizable) {
            splashSize *= ratio;
        }

        QPixmap pixmap;
        if (splashImagePath.endsWith(".svg", Qt::CaseInsensitive)) {
            pixmap = QIcon(splashImagePath).pixmap(splashSize);
        } else {
            pixmap = QPixmap::fromImage(splashImage.scaled(splashSize));
        }
        splash->setPixmap(pixmap);

        for (auto it = configFile.splashSettings.texts.begin(); it != configFile.splashSettings.texts.end(); ++it) {
            const auto &item = it.value();
            SplashScreen::Attribute attr;
            attr.pos = item.pos.size() == 2 ? QPoint(item.pos[0], item.pos[1]) : attr.pos;
            attr.anchor = item.anchor.size() == 2 ? qMakePair(item.anchor[0], item.anchor[1]) : attr.anchor;
            attr.fontSize = item.fontSize > 0 ? item.fontSize : attr.fontSize;
            attr.fontColor = QMCss::CssStringToColor(item.fontColor);
            attr.maxWidth = item.maxWidth > 0 ? item.maxWidth : attr.maxWidth;
            attr.text = item.text;

            if (configFile.resizable) {
                attr.pos *= ratio;
                attr.anchor.first *= ratio;
                attr.anchor.second *= ratio;
                attr.fontSize *= ratio;
                attr.maxWidth *= ratio;
            }

            splash->setTextAttribute(it.key(), attr);
        }
    }

private:
    QString splashImagePath;
    QImage splashImage;
    QSize splashSize;
    QStringList resourcesFiles; // Unused
};

int main_entry(LoaderConfiguration *loadConfig) {
    Q_INIT_RESOURCE(ckloader_res);

    QString workingDir = QDir::currentPath();
    QApplication &a = *qApp;
    QMAppExtension &host = *qAppExt;

    g_loadConfig = loadConfig;

    if (loadConfig->userSettingPath.isEmpty()) {
        loadConfig->userSettingPath = host.appDataDir();
    }

    if (loadConfig->systemSettingPath.isEmpty()) {
        loadConfig->systemSettingPath = host.appShareDir();
    }

    QStringList arguments = a.arguments();
    ArgumentParser argsParser;

    // Process command line arguments
    {
        argsParser.parse(arguments);

        // Root privilege detection
        if (!g_loadConfig->allowRoot && !argsParser.allowRoot && !argsParser.showHelp && QMOs::isUserRoot()) {
            QString msg = QCoreApplication::translate("Application",
                                                      "You're trying to start %1 as the %2, which is "
                                                      "extremely dangerous and therefore strongly not recommended.")
                              .arg(qApp->applicationName(), QMOs::rootUserName());
            qmCon->MsgBox(nullptr, QMCoreConsole::Warning, qApp->applicationName(), msg);
            return false;
        }


        // If need to show help, we simply ignore this error and continue loading plugins
        int code;
        if (!loadConfig->preprocessArguments(arguments, &code) && !argsParser.showHelp) {
            return code;
        }
    }

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, loadConfig->userSettingPath);
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, loadConfig->systemSettingPath);

    PluginManager pluginManager;
    pluginManager.setPluginIID(loadConfig->pluginIID);
    pluginManager.setSettings( //
        new QSettings(QString("%1/%2.plugins.ini").arg(loadConfig->userSettingPath, qApp->applicationName()),
                      QSettings::IniFormat));
    pluginManager.setGlobalSettings( //
        new QSettings(QString("%1/%2.plugins.ini").arg(loadConfig->systemSettingPath, qApp->applicationName()),
                      QSettings::IniFormat));

    SplashScreen splash;
    g_splash = &splash;
    loadConfig->beforeLoadPlugin(&splash);

    ConfigFileLoader configFileLoader;
    configFileLoader.load(loadConfig->splashSettingPath, &splash);

    splash.show();

    // Don't know why drawing text blocks so much time, so we show splash first and then show texts
    splash.showTexts();

    // Update loader text
    splash.showMessage(QCoreApplication::translate("Application", "Searching plugins..."));

    QStringList pluginPaths = loadConfig->pluginPaths + argsParser.customPluginPaths;

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
    for (auto spec : qAsConst(plugins)) {
        if (spec->name() == loadConfig->coreName) {
            coreplugin = spec;
            break;
        }
    }

    QString reason;
    auto loadCorePlugin = [&]() {
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

    if (!loadCorePlugin()) {
        if (argsParser.showHelp) {
            printHelp();
            return 0;
        }
        displayError(msgCoreLoadFailure(reason));
        return 1;
    }

    if (foundAppOptions.contains(QLatin1String(VERSION_OPTION1)) || foundAppOptions.contains(VERSION_OPTION2)) {
        printVersion(coreplugin);
        return 0;
    }
    if (foundAppOptions.contains(QLatin1String(HELP_OPTION1)) ||
        foundAppOptions.contains(QLatin1String(HELP_OPTION2))) {
        printHelp();
        return 0;
    }

    // Init single handle
    SingleApplication single(qApp, true, opts);
    if (!single.isPrimary()) {
        qCDebug(loaderLog) << "primary instance already running. PID:" << single.primaryPid();

        // This eventually needs moved into the NotepadNextApplication to keep
        // sending/receiving logic in the same place
        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);

        stream << PluginManager::serializedArguments();
        single.sendMessage(buffer);

        qCDebug(loaderLog) << "secondary instance closing...";

        return 0;
    } else {
        qCDebug(loaderLog) << "primary instance initializing...";
    }

    // Update loader text
    splash.showMessage(QCoreApplication::translate("Application", "Loading plugins..."));

    PluginManager::loadPlugins();
    if (coreplugin->hasError()) {
        displayError(msgCoreLoadFailure(coreplugin->errorString()));
        return 1;
    }

    loadConfig->afterLoadPlugin();

    // Set up remote arguments.
    QObject::connect(&single, &SingleApplication::receivedMessage, [&](quint32 instanceId, QByteArray message) {
        QDataStream stream(&message, QIODevice::ReadOnly);
        QString msg;
        stream >> msg;
        qCDebug(loaderLog).noquote().nospace() << " remote message from " << instanceId << ", " << msg;
        pluginManager.remoteArguments(msg, nullptr);
    });

    // shutdown plugin manager on the exit
    QObject::connect(&a, &QApplication::aboutToQuit, &pluginManager, &PluginManager::shutdown);

    return Restarter(workingDir).restartOrExit(a.exec());
}
