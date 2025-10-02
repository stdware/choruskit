#ifndef CHORUSKIT_SPLASHCONFIG_H
#define CHORUSKIT_SPLASHCONFIG_H

#include <QStringList>
#include <QMap>

namespace Loader {

    struct SplashText {
        QList<int> pos;
        int alignment = 0;  // Qt::Alignment value
        int fontSize = 0;
        QString fontColor;
        int maxWidth = 0;

        QString text;
    };

    struct SplashSettings {
        QList<int> size;
        QMap<QString, SplashText> texts;
    };

    struct SplashConfig {
        QString splashImage;
        SplashSettings splashSettings;
        QList<int> splashSize;

        bool load(const QString &filename);
    };

}

#endif // CHORUSKIT_SPLASHCONFIG_H
