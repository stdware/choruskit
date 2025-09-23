#ifndef CHORUSKIT_RUNTIMEINTERFACE_H
#define CHORUSKIT_RUNTIMEINTERFACE_H

#include <CoreApi/objectpool.h>

class QDateTime;
class QQmlEngine;
class QJSEngine;
class QSettings;
class QSplashScreen;

namespace Core {

    class RuntimeInterfacePrivate;

    class CKAPPCORE_EXPORT RuntimeInterface : public ObjectPool {
        Q_OBJECT
        Q_DECLARE_PRIVATE(RuntimeInterface)
        Q_PROPERTY(QDateTime startTime READ startTime CONSTANT)
        Q_PROPERTY(QSettings *settings READ settings CONSTANT)
        Q_PROPERTY(QSettings *globalSettings READ globalSettings CONSTANT)
        Q_PROPERTY(QQmlEngine *qmlEngine READ qmlEngine CONSTANT)
    public:
        explicit RuntimeInterface(QObject *parent = nullptr);
        ~RuntimeInterface() override;

        static QDateTime startTime();

        static RuntimeInterface *instance();

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

#endif // CHORUSKIT_RUNTIMEINTERFACE_H
