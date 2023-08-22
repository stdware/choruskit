#include "loaderconfig.h"

bool LoaderConfiguration::preprocessArguments(QStringList &arguments, int *code) {
    return false;
}

void LoaderConfiguration::beforeLoadPlugins(QSplashScreen *screen) {
}

void LoaderConfiguration::afterLoadPlugins() {
}

extern int main_entry(LoaderConfiguration *loadConfig);

int LoaderConfiguration::run() {
    return main_entry(this);
}