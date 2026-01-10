#include "runtimeinterface.h"

#include <CoreApi/private/runtimeinterface_p.h>

#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QSettings>
#include <QTimer>
#include <QtCore/QLoggingCategory>
#include <QtQml/QQmlEngine>

namespace Core {

    Q_STATIC_LOGGING_CATEGORY(lcRuntimeInterface, "ck.runtimeinterface")

    static RuntimeInterface *m_instance = nullptr;

    RuntimeInterfacePrivate::RuntimeInterfacePrivate(RuntimeInterface *q)
        : startTime(QDateTime::currentDateTime()), q_ptr(q) {
    }

    RuntimeInterfacePrivate *RuntimeInterfacePrivate::instance() {
        Q_ASSERT(m_instance);
        return RuntimeInterface::instance()->d_func();
    }

    int RuntimeInterfacePrivate::restartOrExit(int exitCode) {
        return instance()->restartOrExitInternal(exitCode);
    }

    int RuntimeInterfacePrivate::restartOrExitInternal(int exitCode) {
        qCInfo(lcRuntimeInterface) << "QApplication exited with code" << exitCode;
        runExitCallbacks(exitCode);
        if (!restartPlanned) {
            qCInfo(lcRuntimeInterface) << "Goodbye";
            return exitCode;
        }

        qint64 pid = -1;
        const bool started = QProcess::startDetached(QApplication::applicationFilePath(),
                                                     QApplication::arguments(), QDir::currentPath(),
                                                     &pid);

        if (started) {
            qCInfo(lcRuntimeInterface) << "Restarting application new process pid" << pid;
        } else {
            qCWarning(lcRuntimeInterface) << "Failed to restart application";
        }

        return exitCode;
    }

    void RuntimeInterfacePrivate::setRestartPlanned(bool planned) {
        if (restartPlanned == planned) {
            return;
        }

        restartPlanned = planned;
        if (auto *q = q_func()) {
            Q_EMIT q->restartPlannedChanged();
        }
    }

    bool RuntimeInterfacePrivate::isRestartPlanned() const {
        return restartPlanned;
    }

    void RuntimeInterfacePrivate::addExitCallback(const std::function<void(int)> &callback) {
        exitCallbacks.push_back(callback);
    }

    void RuntimeInterfacePrivate::runExitCallbacks(int exitCode) const {
        for (const auto &callback : exitCallbacks) {
            if (callback) {
                callback(exitCode);
            }
        }
    }

    RuntimeInterface::RuntimeInterface(QObject *parent)
        : ObjectPool(parent), d_ptr(new RuntimeInterfacePrivate(this)) {
        Q_ASSERT(!m_instance);
        m_instance = this;
    }

    RuntimeInterface::~RuntimeInterface() {
        m_instance = nullptr;
    }

    QDateTime RuntimeInterface::startTime() {
        return RuntimeInterfacePrivate::instance()->startTime;
    }

    QSettings *RuntimeInterface::settings() {
        return RuntimeInterfacePrivate::instance()->settings;
    }

    void RuntimeInterface::setSettings(QSettings *settings) {
        auto *d = RuntimeInterfacePrivate::instance();
        auto &settingsRef = d->settings;
        delete settingsRef;

        settingsRef = settings;
        settings->setParent(m_instance);
    }

    QSettings *RuntimeInterface::globalSettings() {
        return RuntimeInterfacePrivate::instance()->globalSettings;
    }

    void RuntimeInterface::setGlobalSettings(QSettings *settings) {
        auto *d = RuntimeInterfacePrivate::instance();
        auto &settingsRef = d->globalSettings;
        delete settingsRef;

        settingsRef = settings;
        settings->setParent(m_instance);
    }

    QQmlEngine *RuntimeInterface::qmlEngine() {
        return RuntimeInterfacePrivate::instance()->qmlEngine;
    }

    void RuntimeInterface::setQmlEngine(QQmlEngine *qmlEngine) {
        RuntimeInterfacePrivate::instance()->qmlEngine = qmlEngine;
    }

    QSplashScreen *RuntimeInterface::splash() {
        return RuntimeInterfacePrivate::instance()->splash;
    }

    void RuntimeInterface::setSplash(QSplashScreen *splash) {
        auto *d = RuntimeInterfacePrivate::instance();
        auto &splashRef = d->splash;
        Q_ASSERT(!splashRef);
        splashRef = splash;
    }

    Logger *RuntimeInterface::logger() {
        return RuntimeInterfacePrivate::instance()->logger;
    }

    void RuntimeInterface::setLogger(Logger *logger) {
        RuntimeInterfacePrivate::instance()->logger = logger;
    }

    TranslationManager *RuntimeInterface::translationManager() {
        return RuntimeInterfacePrivate::instance()->translationManager;
    }

    void RuntimeInterface::setTranslationManager(TranslationManager *translationManager) {
        RuntimeInterfacePrivate::instance()->translationManager = translationManager;
    }

    void RuntimeInterface::exitApplicationGracefully(int exitCode) {
        Q_ASSERT(m_instance);
        qCInfo(lcRuntimeInterface) << "Manually exit" << exitCode;
        QTimer::singleShot(0, [exitCode] {
            qCDebug(lcRuntimeInterface) << "Calling QCoreApplication::exit" << exitCode;
            QCoreApplication::exit(exitCode);
        });
    }

    void RuntimeInterface::restartApplication(int exitCode) {
        Q_ASSERT(m_instance);
        qCInfo(lcRuntimeInterface) << "Scheduling restart" << exitCode;
        RuntimeInterfacePrivate::instance()->setRestartPlanned(true);
        exitApplicationGracefully(exitCode);
    }

    bool RuntimeInterface::restartPlanned() {
        return RuntimeInterfacePrivate::instance()->isRestartPlanned();
    }

    void RuntimeInterface::addExitCallback(const std::function<void(int)> &callback) {
        RuntimeInterfacePrivate::instance()->addExitCallback(callback);
    }

    RuntimeInterface *RuntimeInterface::instance() {
        return m_instance;
    }

}

#include "moc_runtimeinterface.cpp"
