#ifndef CHORUSKIT_PLUGINDATABASE_H
#define CHORUSKIT_PLUGINDATABASE_H

#include <CoreApi/objectpool.h>

class QDateTime;
class QQmlEngine;
class QJSEngine;
class QSettings;
class QSplashScreen;

namespace Core {

    class PluginDatabasePrivate;

    class CKAPPCORE_EXPORT PluginDatabase : public ObjectPool {
        Q_OBJECT
        Q_DECLARE_PRIVATE(PluginDatabase)
        Q_PROPERTY(QDateTime startTime READ startTime CONSTANT)
        Q_PROPERTY(QSettings *settings READ settings CONSTANT)
        Q_PROPERTY(QSettings *globalSettings READ globalSettings CONSTANT)
        Q_PROPERTY(QQmlEngine *qmlEngine READ qmlEngine CONSTANT)
    public:
        explicit PluginDatabase(QObject *parent = nullptr);
        ~PluginDatabase() override;

        static QDateTime startTime();

        static PluginDatabase *instance();

    public:
        static QSettings *settings();
        static void setSettings(QSettings *settings);

        static QSettings *globalSettings();
        static void setGlobalSettings(QSettings *settings);

        static QQmlEngine *qmlEngine();
        static void setQmlEngine(QQmlEngine *qmlEngine);

        static QSplashScreen *splash();
        static void setSplash(QSplashScreen *splash);
    };

}

#endif // CHORUSKIT_PLUGINDATABASE_H
