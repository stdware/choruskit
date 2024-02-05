#include "loaderconfig.h"

extern int main_entry(Loader::LoaderConfiguration *loadConfig);

namespace Loader {

    bool LoaderConfiguration::preprocessArguments(QStringList &arguments, int *code) {
        return false;
    }

    void LoaderConfiguration::beforeLoadPlugins(QSplashScreen *screen) {
    }

    void LoaderConfiguration::afterLoadPlugins() {
    }

    int LoaderConfiguration::run() {
        return main_entry(this);
    }

}