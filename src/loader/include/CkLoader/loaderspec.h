#ifndef CHORUSKIT_LOADERSPEC_H
#define CHORUSKIT_LOADERSPEC_H

#include <QtCore/QPair>
#include <QtCore/QStringList>
#include <QtCore/QSettings>
#include <QtWidgets/QSplashScreen>

namespace Loader {

    class LoaderSpec {
    public:
        /// Whether to allow only a single process instance
        bool single;

        /// Add an \a --allow-root option, display warning and exit if running
        /// as \c Root/Administrator without setting this option
        bool allowRoot;

        /// Core plugin name, default to \c Core
        QString coreName;

        /// Plugin Interface ID
        QString pluginIID;

        /// Default plugin searching paths
        QStringList pluginPaths;

        /// Splash configuration file path
        QString splashConfigPath;

        struct Argument {
            QStringList options;
            QString param;
            QString description;
        };
        /// Other options to display in help text
        QList<Argument> extraArguments;
        
        /// Create a new settings for QtCreator ExtensionSystem
        virtual QSettings *createExtensionSystemSettings(QSettings::Scope scope);

        /// Create a new settings for ChorusKit
        virtual QSettings *createChorusKitSettings(QSettings::Scope scope);
 
        /// Parse extra options and do some initializations
        virtual bool preprocessArguments(QStringList &arguments, int *code = nullptr);

        /// Show text on splash
        virtual void splashWillShow(QSplashScreen *screen);

        /// Show splash
        virtual void splashShown(QSplashScreen *screen);

        /// Do something before loading all plugins
        virtual void beforeLoadPlugins();

        /// Do something after loading all plugins
        virtual void afterLoadPlugins();

        /// Load plugins and run application
        /// \note Create `QApplication` instance before calling it (Important!!!)
        int run();

    public:
        /// Display error message and exit with \a exitCode
        static void displayError(const QString &err, int exitCode = -1);

    public:
        inline LoaderSpec() : single(true), allowRoot(true), coreName("Core") {
        }
        virtual ~LoaderSpec() = default;
    };

}

#endif // CHORUSKIT_LOADERSPEC_H
