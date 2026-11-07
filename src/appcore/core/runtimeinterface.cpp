#include "runtimeinterface.h"
#include "runtimeinterface_p.h"

#include <utility>

#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QList>
#include <QPointer>
#include <QProcess>
#include <QScopeGuard>
#include <QSettings>
#include <QTimer>
#include <QWindow>
#include <QtCore/QLoggingCategory>
#include <QtQml/QQmlEngine>

#include <CoreApi/FileOpenEventSource.h>

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

    void RuntimeInterfacePrivate::requestExit(int exitCode, bool restart) {
        Q_Q(RuntimeInterface);
        if (exitInProgress) {
            return;
        }
        exitInProgress = true;
        QTimer::singleShot(0, q, [this, exitCode, restart] {
            const bool quitOnLastWindowClosed = QApplication::quitOnLastWindowClosed();
            QApplication::setQuitOnLastWindowClosed(false);
            const auto restoreQuitOnLastWindowClosed = qScopeGuard([quitOnLastWindowClosed] {
                QApplication::setQuitOnLastWindowClosed(quitOnLastWindowClosed);
            });

            QList<QPointer<QWindow>> windows;
            for (auto *window : QApplication::topLevelWindows()) {
                windows.append(window);
            }
            for (const auto &window : std::as_const(windows)) {
                if (window && !window->close()) {
                    exitInProgress = false;
                    qCInfo(lcRuntimeInterface) << "Exit canceled because a window refused closure.";
                    return;
                }
            }

            QTimer::singleShot(0, q_ptr, [this, exitCode, restart] {
                for (auto *window : QApplication::topLevelWindows()) {
                    if (window->isVisible()) {
                        exitInProgress = false;
                        qCInfo(lcRuntimeInterface) << "Exit canceled because a window remains visible.";
                        return;
                    }
                }
                setRestartPlanned(restart);
                qCDebug(lcRuntimeInterface) << "Calling QCoreApplication::exit" << exitCode;
                QCoreApplication::exit(exitCode);
            });
        });
    }

    bool RuntimeInterfacePrivate::isExitInProgress() const {
        return exitInProgress;
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
        Q_D(RuntimeInterface);
        d->fileOpenEventSource = new FileOpenEventSource(this);
        qApp->installEventFilter(this);
    }

    RuntimeInterface::~RuntimeInterface() {
        m_instance = nullptr;
    }

    QDateTime RuntimeInterface::startTime() {
        return RuntimeInterfacePrivate::instance()->startTime;
    }

    FileOpenEventSource *RuntimeInterface::fileOpenEventSource() {
        return RuntimeInterfacePrivate::instance()->fileOpenEventSource;
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

    QSettings *RuntimeInterface::state() {
        return RuntimeInterfacePrivate::instance()->state;
    }

    void RuntimeInterface::setState(QSettings *state) {
        auto *d = RuntimeInterfacePrivate::instance();
        auto &stateRef = d->state;
        delete stateRef;

        stateRef = state;
        state->setParent(m_instance);
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
        RuntimeInterfacePrivate::instance()->requestExit(exitCode, false);
    }

    void RuntimeInterface::restartApplication(int exitCode) {
        Q_ASSERT(m_instance);
        qCInfo(lcRuntimeInterface) << "Scheduling restart" << exitCode;
        RuntimeInterfacePrivate::instance()->requestExit(exitCode, true);
    }

    bool RuntimeInterface::exitInProgress() {
        return RuntimeInterfacePrivate::instance()->isExitInProgress();
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

    bool RuntimeInterface::eventFilter(QObject *watched, QEvent *event) {
        if (watched == qApp && event->type() == QEvent::Quit) {
            event->ignore();
            exitApplicationGracefully();
            return true;
        }
        return ObjectPool::eventFilter(watched, event);
    }

}

#include "moc_runtimeinterface.cpp"
