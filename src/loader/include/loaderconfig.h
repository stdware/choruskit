#ifndef ENTRY_H
#define ENTRY_H

#include <QPair>
#include <QSplashScreen>
#include <QStringList>

class LoaderConfiguration {
public:
    bool allowRoot;

    QString coreName;
    QString pluginIID;
    QStringList pluginPaths;

    QString splashSettingPath;

    struct Argument {
        QStringList options;
        QString param;
        QString description;
    };
    QList<Argument> extraArguments;

    QString userSettingPath;
    QString systemSettingPath;

    inline LoaderConfiguration() : allowRoot(true), coreName("Core"){};
    virtual ~LoaderConfiguration(){};

    virtual bool preprocessArguments(QStringList &arguments, int *code = nullptr);
    virtual void beforeLoadPlugin(QSplashScreen *screen);
    virtual void afterLoadPlugin();

    int run();
};

#endif // ENTRY_H
