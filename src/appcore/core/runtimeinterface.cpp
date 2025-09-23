#include "runtimeinterface.h"

#include <tuple>

#include <QDateTime>
#include <QSettings>
#include <QtQml/QQmlEngine>

#include "objectpool_p.h"

namespace Core {

#define myWarning (qWarning().nospace() << "Core::RuntimeInterface::" << __func__ << "():").space()

#define checkInstanceV                                                                             \
    if (!m_instance) {                                                                             \
        myWarning << "Please instantiate the RuntimeInterface object first";                         \
        return;                                                                                    \
    }

#define checkInstance                                                                              \
    if (!m_instance) {                                                                             \
        myWarning << "Please instantiate the RuntimeInterface object first";                         \
        return {};                                                                                 \
    }

    class RuntimeInterfacePrivate : public ObjectPoolPrivate {
        Q_DECLARE_PUBLIC(RuntimeInterface)
    public:
        RuntimeInterfacePrivate() {
            std::ignore = RuntimeInterface::startTime();
        }

        QSettings *settings = nullptr;
        QSettings *globalSettings = nullptr;

        QSplashScreen *splash = nullptr;

        QQmlEngine *qmlEngine = nullptr;
    };

    static RuntimeInterface *m_instance = nullptr;

    RuntimeInterface::RuntimeInterface(QObject *parent)
        : ObjectPool(*new RuntimeInterfacePrivate(), parent) {
        Q_ASSERT(!m_instance);
        m_instance = this;
    }

    RuntimeInterface::~RuntimeInterface() {
        m_instance = nullptr;
    }

    QDateTime RuntimeInterface::startTime() {
        static QDateTime start = QDateTime::currentDateTime();
        return start;
    }

    QSettings *RuntimeInterface::settings() {
        checkInstance;
        return m_instance->d_func()->settings;
    }

    void RuntimeInterface::setSettings(QSettings *settings) {
        checkInstanceV;

        auto &settingsRef = m_instance->d_func()->settings;
        delete settingsRef;

        settingsRef = settings;
        settings->setParent(m_instance);
    }

    QSettings *RuntimeInterface::globalSettings() {
        checkInstance;
        return m_instance->d_func()->globalSettings;
    }

    void RuntimeInterface::setGlobalSettings(QSettings *settings) {
        checkInstanceV;

        auto &settingsRef = m_instance->d_func()->globalSettings;
        delete settingsRef;

        settingsRef = settings;
        settings->setParent(m_instance);
    }

    QQmlEngine *RuntimeInterface::qmlEngine() {
        checkInstance;
        return m_instance->d_func()->qmlEngine;
    }

    void RuntimeInterface::setQmlEngine(QQmlEngine *qmlEngine) {
        checkInstanceV;
        m_instance->d_func()->qmlEngine = qmlEngine;
    }

    QSplashScreen *RuntimeInterface::splash() {
        checkInstance;
        return m_instance->d_func()->splash;
    }

    void RuntimeInterface::setSplash(QSplashScreen *splash) {
        checkInstanceV;

        auto &splashRef = m_instance->d_func()->splash;
        if (splashRef) {
            myWarning << "Ignores the new splash screen because there is already one set";
            return;
        }
        splashRef = splash;
    }

    RuntimeInterface *RuntimeInterface::instance() {
        return m_instance;
    }

}

#include "moc_runtimeinterface.cpp"
