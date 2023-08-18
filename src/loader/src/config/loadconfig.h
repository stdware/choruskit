#ifndef LOADCONFIG_H
#define LOADCONFIG_H

#include <QStringList>

#include "qjsonstream.h"

struct SplashText {
    QAS_JSON(SplashText)

    QList<int> pos;
    QList<int> anchor;
    int fontSize;
    QString fontColor;
    int maxWidth;

    QString text;

    SplashText();
};

struct SplashSettings {
    QAS_JSON(SplashSettings)

    QList<int> size;
    QMap<QString, SplashText> texts;
};

struct LoadConfig {
    QAS_JSON(LoadConfig)

    QString splashImage;
    SplashSettings splashSettings;
    QStringList resourceFiles;
    bool resizable;
    QList<int> splashSize;

    bool load(const QString &filename);
};


#endif // LOADCONFIG_H
