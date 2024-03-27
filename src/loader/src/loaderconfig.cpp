#include "loaderconfig.h"

extern int main_entry(Loader::LoaderConfiguration *loadConfig);

extern void displayError(const QString &t, int exitCode = -1);

namespace Loader {

    bool LoaderConfiguration::preprocessArguments(QStringList &arguments, int *code) {
        return true;
    }

    void LoaderConfiguration::splashWillShow(QSplashScreen *screen) {
    }

    void LoaderConfiguration::beforeLoadPlugins() {
    }

    void LoaderConfiguration::afterLoadPlugins() {
    }

    int LoaderConfiguration::run() {
        return main_entry(this);
    }

    void LoaderConfiguration::showError(const QString &err, int exitCode) {
        displayError(err, exitCode);
    }

}