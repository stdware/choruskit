#ifndef CHORUSKIT_PLUGINDATABASE_H
#define CHORUSKIT_PLUGINDATABASE_H

#include <QtCore/QSettings>
#include <QtCore/QDateTime>
#include <QtWidgets/QSplashScreen>

#include <CoreApi/objectpool.h>

namespace Core {

    class PluginDatabasePrivate;

    class CKAPPCORE_EXPORT PluginDatabase : public ObjectPool {
        Q_OBJECT
        Q_DECLARE_PRIVATE(PluginDatabase)
    public:
        explicit PluginDatabase(QObject *parent = nullptr);
        ~PluginDatabase();

        static QDateTime startTime();

        static PluginDatabase *instance();

    public:
        static QSettings *settings();
        static void setSettings(QSettings *settings);

        static QSettings *globalSettings();
        static void setGlobalSettings(QSettings *settings);

        static QSplashScreen *splash();
        static void setSplash(QSplashScreen *splash);
    };

}

#endif // CHORUSKIT_PLUGINDATABASE_H