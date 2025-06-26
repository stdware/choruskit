#include "loaderspec.h"

#include <QtCore/QCoreApplication>

#include <CoreApi/applicationinfo.h>

extern int __main__(Loader::LoaderSpec *loadConfig);

extern void displayError(const QString &t, int exitCode = -1);

namespace Loader {

    using namespace Core;

    QSettings *LoaderSpec::createExtensionSystemSettings(bool global) {
        if (global) {
            return new QSettings(
                QStringLiteral("%1/%2.extensionsystem.ini")
                    .arg(ApplicationInfo::applicationLocation(ApplicationInfo::BuiltinResources),
                         QCoreApplication::applicationName()),
                QSettings::IniFormat);
        } else {
            return new QSettings(
                QStringLiteral("%1/%2.extensionsystem.ini")
                    .arg(ApplicationInfo::applicationLocation(ApplicationInfo::RuntimeData),
                         QCoreApplication::applicationName()),
                QSettings::IniFormat);
        }
    }

    /// Create a new settings for ChorusKit
    QSettings *LoaderSpec::createChorusKitSettings(bool global) {
        if (global) {
            return new QSettings(
                QString("%1/%2.plugins.ini")
                    .arg(ApplicationInfo::applicationLocation(ApplicationInfo::BuiltinResources),
                         QCoreApplication::applicationName()),
                QSettings::IniFormat);
        } else {
            return new QSettings(
                QString("%1/%2.plugins.ini")
                    .arg(ApplicationInfo::applicationLocation(ApplicationInfo::RuntimeData),
                         QCoreApplication::applicationName()),
                QSettings::IniFormat);
        }
    }

    bool LoaderSpec::preprocessArguments(QStringList &arguments, int *code) {
        return true;
    }

    void LoaderSpec::splashWillShow(QSplashScreen *screen) {
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