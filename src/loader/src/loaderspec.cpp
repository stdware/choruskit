#include "loaderspec.h"

#include <QtCore/QCoreApplication>

#include <CoreApi/applicationinfo.h>

extern int __main__(Loader::LoaderSpec *loadConfig);

extern void displayError(const QString &t, int exitCode = -1);

namespace Loader {

    using namespace Core;

    QSettings *LoaderSpec::createExtensionSystemSettings(QSettings::Scope scope) {
        QString dir = ApplicationInfo::applicationLocation(scope == QSettings::UserScope
                                                               ? ApplicationInfo::RuntimeData
                                                               : ApplicationInfo::BuiltinResources);
        return new QSettings(QStringLiteral("%1/%2.extensionsystem.ini")
                                 .arg(dir, QCoreApplication::applicationName()),
                             QSettings::IniFormat);
    }

    /// Create a new settings for ChorusKit
    QSettings *LoaderSpec::createChorusKitSettings(QSettings::Scope scope) {
        QString dir = ApplicationInfo::applicationLocation(scope == QSettings::UserScope
                                                               ? ApplicationInfo::RuntimeData
                                                               : ApplicationInfo::BuiltinResources);
        return new QSettings(
            QStringLiteral("%1/%2.plugins.ini").arg(dir, QCoreApplication::applicationName()),
            QSettings::IniFormat);
    }


    bool LoaderSpec::preprocessArguments(QStringList &arguments, int *code) {
        return true;
    }

    void LoaderSpec::splashWillShow(QSplashScreen *screen) {
    }

    void LoaderSpec::splashShown(QSplashScreen *screen) {
    }

    void LoaderSpec::beforeLoadPlugins() {
    }

    void LoaderSpec::afterLoadPlugins() {
    }

    int LoaderSpec::run() {
        return __main__(this);
    }

    void LoaderSpec::displayError(const QString &err, int exitCode) {
        ::displayError(err, exitCode);
    }

}
