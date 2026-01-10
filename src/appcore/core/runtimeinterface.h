#ifndef CHORUSKIT_RUNTIMEINTERFACE_H
#define CHORUSKIT_RUNTIMEINTERFACE_H

#include <functional>

#include <QScopedPointer>

#include <CoreApi/objectpool.h>

class QDateTime;
class QQmlEngine;
class QJSEngine;
class QSettings;
class QSplashScreen;

namespace Core {

    class Logger;
    class TranslationManager;

    class RuntimeInterfacePrivate;

    class CKAPPCORE_EXPORT RuntimeInterface : public ObjectPool {
        Q_OBJECT
        Q_DECLARE_PRIVATE(RuntimeInterface)
        Q_PROPERTY(QDateTime startTime READ startTime CONSTANT)
        Q_PROPERTY(QSettings *settings READ settings CONSTANT)
        Q_PROPERTY(QSettings *globalSettings READ globalSettings CONSTANT)
        Q_PROPERTY(QQmlEngine *qmlEngine READ qmlEngine CONSTANT)
        Q_PROPERTY(bool restartPlanned READ restartPlanned NOTIFY restartPlannedChanged)
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

        static Logger *logger();
        static void setLogger(Logger *logger);

        static TranslationManager *translationManager();
        static void setTranslationManager(TranslationManager *translationManager);

        Q_INVOKABLE static void exitApplicationGracefully(int exitCode = 0);
        Q_INVOKABLE static void restartApplication(int exitCode = 0);

        static bool restartPlanned();

        static void addExitCallback(const std::function<void(int)> &callback);

    Q_SIGNALS:
        void restartPlannedChanged();

    private:
        QScopedPointer<RuntimeInterfacePrivate> d_ptr;
    };

}

#endif // CHORUSKIT_RUNTIMEINTERFACE_H
