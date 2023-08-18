#ifndef ENTRY_H
#define ENTRY_H

#include <QPair>
#include <QSplashScreen>
#include <QStringList>

class LoaderConfiguration {
public:
    // Add an `--allow-root` option, display warning and exit if running
    // as Root User/Administrator without setting this option
    bool allowRoot;

    QString coreName;          // Core plugin name, default to "Core"
    QString pluginIID;         // Plugin Interface ID
    QStringList pluginPaths;   // Default plugin searching paths

    QString splashSettingPath; // Splash configuration file path

    struct Argument {
        QStringList options;
        QString param;
        QString description;
    };
    QList<Argument> extraArguments; // Other options to display in help text

    QString userSettingsPath;       // Plugin manager user settings directory
    QString systemSettingsPath;     // Plugin manager global settings directory

    inline LoaderConfiguration() : allowRoot(true), coreName("Core"){};
    virtual ~LoaderConfiguration(){};

    // Parse extra options and do some initializations
    virtual bool preprocessArguments(QStringList &arguments, int *code = nullptr);

    // You may need to show text on splash
    virtual void beforeLoadPlugin(QSplashScreen *screen);

    // You may need to do some final jobs after loading all plugins
    virtual void afterLoadPlugin();

    // Create `QApplication` and `QMAppExtension` instances before calling this function
    int run();
};

#endif // ENTRY_H
