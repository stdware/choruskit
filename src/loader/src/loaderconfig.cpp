#include "loaderconfig.h"

bool LoaderConfiguration::preprocessArguments(QStringList &arguments, int *code) {
    return false;
}

void LoaderConfiguration::beforeLoadPlugin(QSplashScreen *screen) {
    // Do nothing
}

void LoaderConfiguration::afterLoadPlugin() {
}

extern int main_entry(LoaderConfiguration *loadConfig);

int LoaderConfiguration::run() {
    return main_entry(this);
}