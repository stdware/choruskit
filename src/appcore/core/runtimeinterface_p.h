#ifndef CHORUSKIT_RUNTIMEINTERFACE_P_H
#define CHORUSKIT_RUNTIMEINTERFACE_P_H

#include <CoreApi/runtimeinterface.h>

#include <functional>
#include <vector>

#include <QDateTime>
#include <QtQmlIntegration/qqmlintegration.h>

class QQmlEngine;
class QJSEngine;

namespace Core {

    class CKAPPCORE_EXPORT RuntimeInterfacePrivate {
        Q_DECLARE_PUBLIC(RuntimeInterface)
    public:
        explicit RuntimeInterfacePrivate(RuntimeInterface *q);

        static RuntimeInterfacePrivate *instance();
        static int restartOrExit(int exitCode);

        int restartOrExitInternal(int exitCode);

        void requestExit(int exitCode, bool restart);
        bool isExitInProgress() const;

        void setRestartPlanned(bool planned);
        bool isRestartPlanned() const;

        void addExitCallback(const std::function<void(int)> &callback);

        QDateTime startTime;

        RuntimeInterface *q_ptr;

        FileOpenEventSource *fileOpenEventSource = nullptr;

        QSettings *settings = nullptr;
        QSettings *state = nullptr;
        QSettings *globalSettings = nullptr;

        QSplashScreen *splash = nullptr;

        QQmlEngine *qmlEngine = nullptr;

        Logger *logger = nullptr;
        TranslationManager *translationManager = nullptr;

    private:
        void runExitCallbacks(int exitCode) const;

        bool exitInProgress = false;
        bool restartPlanned = false;
        std::vector<std::function<void(int)>> exitCallbacks;
    };

    struct RuntimeInterfaceForeign {
        Q_GADGET
        QML_FOREIGN(RuntimeInterface)
        QML_SINGLETON
        QML_NAMED_ELEMENT(RuntimeInterface)
    public:

        static inline RuntimeInterface *create(QQmlEngine *, QJSEngine *) {
            return RuntimeInterface::instance();
        }
    };
}

#endif // CHORUSKIT_RUNTIMEINTERFACE_P_H
