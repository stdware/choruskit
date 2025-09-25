#include "runtimeinterface.h"

#include <tuple>

#include <QDateTime>
#include <QSettings>
#include <QtQml/QQmlEngine>

#include "objectpool_p.h"

namespace Core {

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

        Logger *logger = nullptr;
        TranslationManager *translationManager = nullptr;
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
        Q_ASSERT(m_instance);
        return m_instance->d_func()->settings;
    }

    void RuntimeInterface::setSettings(QSettings *settings) {
        Q_ASSERT(m_instance);
        auto &settingsRef = m_instance->d_func()->settings;
        delete settingsRef;

        settingsRef = settings;
        settings->setParent(m_instance);
    }

    QSettings *RuntimeInterface::globalSettings() {
        Q_ASSERT(m_instance);
        return m_instance->d_func()->globalSettings;
    }

    void RuntimeInterface::setGlobalSettings(QSettings *settings) {
        Q_ASSERT(m_instance);
        auto &settingsRef = m_instance->d_func()->globalSettings;
        delete settingsRef;

        settingsRef = settings;
        settings->setParent(m_instance);
    }

    QQmlEngine *RuntimeInterface::qmlEngine() {
        Q_ASSERT(m_instance);
        return m_instance->d_func()->qmlEngine;
    }

    void RuntimeInterface::setQmlEngine(QQmlEngine *qmlEngine) {
        Q_ASSERT(m_instance);
        m_instance->d_func()->qmlEngine = qmlEngine;
    }

    QSplashScreen *RuntimeInterface::splash() {
        Q_ASSERT(m_instance);
        return m_instance->d_func()->splash;
    }

    void RuntimeInterface::setSplash(QSplashScreen *splash) {
        Q_ASSERT(m_instance);
        auto &splashRef = m_instance->d_func()->splash;
        Q_ASSERT(!splashRef);
        splashRef = splash;
    }

    Logger * RuntimeInterface::logger() {
        Q_ASSERT(m_instance);
        return m_instance->d_func()->logger;
    }

    void RuntimeInterface::setLogger(Logger *logger) {
        Q_ASSERT(m_instance);
        m_instance->d_func()->logger = logger;
    }
    TranslationManager *RuntimeInterface::translationManager() {
        Q_ASSERT(m_instance);
        return m_instance->d_func()->translationManager;
    }
    void RuntimeInterface::setTranslationManager(TranslationManager *translationManager) {
        Q_ASSERT(m_instance);
        m_instance->d_func()->translationManager = translationManager;
    }

    RuntimeInterface *RuntimeInterface::instance() {
        return m_instance;
    }

}

#include "moc_runtimeinterface.cpp"
