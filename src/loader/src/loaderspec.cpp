#include "loaderspec.h"

extern int __main__(Loader::LoaderSpec *loadConfig);

extern void displayError(const QString &t, int exitCode = -1);

namespace Loader {

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