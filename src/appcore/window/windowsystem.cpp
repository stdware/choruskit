#include "windowsystem.h"
#include "windowsystem_p.h"

#include <algorithm>

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QPointer>
#include <QScreen>
#include <QWindow>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include <QTimer>
#include <QQmlInfo>
#include <QLoggingCategory>

#include "runtimeinterface.h"

namespace Core {

    Q_STATIC_LOGGING_CATEGORY(lcWindowSystem, "ck.windowsystem")

    WindowGeometry WindowGeometry::fromObject(const QJsonObject &obj) {
        QRect winRect;
        winRect.setX(obj.value("x").toInt());
        winRect.setY(obj.value("y").toInt());
        winRect.setWidth(obj.value("width").toInt());
        winRect.setHeight(obj.value("height").toInt());

        bool isMax = obj.value("isMaximized").toBool();

        return {winRect, isMax};
    }

    QJsonObject WindowGeometry::toObject() const {
        QJsonObject obj;
        obj.insert("x", geometry.x());
        obj.insert("y", geometry.y());
        obj.insert("width", geometry.width());
        obj.insert("height", geometry.height());
        obj.insert("isMaximized", maximized);
        return obj;
    }

    WindowSystemAttachedType::WindowSystemAttachedType(QObject *parent)
        : QObject(parent), m_window(qobject_cast<QWindow *>(parent)) {
        // Create debounce timer
        m_saveTimer = new QTimer(this);
        m_saveTimer->setSingleShot(true);
        m_saveTimer->setInterval(500); // 500ms debounce
        connect(m_saveTimer, &QTimer::timeout, this, &WindowSystemAttachedType::saveGeometryDebounced);
    }

    WindowSystemAttachedType::~WindowSystemAttachedType() {
        disconnectFromWindow();
    }

    WindowSystemAttachedType *WindowSystem::qmlAttachedProperties(QObject *object) {
        QWindow *window = qobject_cast<QWindow *>(object);
        if (!window)
            qmlWarning(object) << "WindowSystem should be attached to a QWindow";
        return new WindowSystemAttachedType(object);
    }

    WindowSystem *WindowSystemAttachedType::windowSystem() const {
        return m_windowSystem;
    }

    void WindowSystemAttachedType::setWindowSystem(WindowSystem *windowSystem) {
        if (m_windowSystem != windowSystem) {
            m_windowSystem = windowSystem;
            Q_EMIT windowSystemChanged();
            loadGeometryIfReady();
        }
    }

    QString WindowSystemAttachedType::id() const {
        return m_id;
    }

    void WindowSystemAttachedType::setId(const QString &id) {
        if (m_id != id) {
            m_id = id;
            Q_EMIT idChanged();
            loadGeometryIfReady();
        }
    }

    void WindowSystemAttachedType::connectToWindow() {
        if (!m_window) {
            return;
        }

        connect(m_window, &QWindow::xChanged, this, &WindowSystemAttachedType::onWindowGeometryChanged);
        connect(m_window, &QWindow::yChanged, this, &WindowSystemAttachedType::onWindowGeometryChanged);
        connect(m_window, &QWindow::widthChanged, this, &WindowSystemAttachedType::onWindowGeometryChanged);
        connect(m_window, &QWindow::heightChanged, this, &WindowSystemAttachedType::onWindowGeometryChanged);
        connect(m_window, &QWindow::visibilityChanged, this, &WindowSystemAttachedType::onWindowVisibilityChanged);
    }

    void WindowSystemAttachedType::disconnectFromWindow() {
        if (!m_window) {
            return;
        }

        disconnect(m_window, &QWindow::xChanged, this, &WindowSystemAttachedType::onWindowGeometryChanged);
        disconnect(m_window, &QWindow::yChanged, this, &WindowSystemAttachedType::onWindowGeometryChanged);
        disconnect(m_window, &QWindow::widthChanged, this, &WindowSystemAttachedType::onWindowGeometryChanged);
        disconnect(m_window, &QWindow::heightChanged, this, &WindowSystemAttachedType::onWindowGeometryChanged);
        disconnect(m_window, &QWindow::visibilityChanged, this, &WindowSystemAttachedType::onWindowVisibilityChanged);
    }

    void WindowSystemAttachedType::loadGeometryIfReady() {
        if (!m_windowSystem || m_id.isEmpty() || !m_window) {
            return;
        }

        // Connect to window signals when we have both windowSystem and id
        connectToWindow();
        
        // Load geometry
        m_windowSystem->loadGeometry(m_id, m_window, m_window->size());
    }

    void WindowSystemAttachedType::onWindowGeometryChanged() const {
        if (!m_windowSystem || m_id.isEmpty() || !m_window) {
            return;
        }

        // Start/restart debounce timer
        m_saveTimer->start();
    }

    void WindowSystemAttachedType::onWindowVisibilityChanged() const {
        if (!m_windowSystem || m_id.isEmpty() || !m_window) {
            return;
        }

        // Start/restart debounce timer
        m_saveTimer->start();
    }

    void WindowSystemAttachedType::saveGeometryDebounced() const {
        if (!m_windowSystem || m_id.isEmpty() || !m_window) {
            return;
        }

        m_windowSystem->saveGeometry(m_id, m_window);
    }

    WindowSystemPrivate::WindowSystemPrivate() : q_ptr(nullptr) {
    }

    WindowSystemPrivate::~WindowSystemPrivate() {
    }

    void WindowSystemPrivate::init() {
        readSettings();
    }

    static const char settingCatalogC[] = "WindowSystem";

    static const char winGeometryGroupC[] = "WindowGeometry";

    void WindowSystemPrivate::readSettings() {
        winGeometries.clear();

        auto settings = RuntimeInterface::settings();
        settings->beginGroup(QLatin1String(settingCatalogC));

        auto winPropsObj = settings->value(winGeometryGroupC).toJsonObject();
        for (auto it = winPropsObj.begin(); it != winPropsObj.end(); ++it) {
            if (!it->isObject()) {
                continue;
            }
            winGeometries.insert(it.key(), WindowGeometry::fromObject(it->toObject()));
        }

        shouldStoreGeometry = settings->value("shouldStoreGeometry", true).toBool();

        settings->endGroup();
    }

    void WindowSystemPrivate::saveSettings() const {
        QJsonObject winPropsObj;
        for (auto it = winGeometries.begin(); it != winGeometries.end(); ++it) {
            winPropsObj.insert(it.key(), it->toObject());
        }

        auto settings = RuntimeInterface::settings();
        settings->beginGroup(QLatin1String(settingCatalogC));

        settings->setValue(QLatin1String(winGeometryGroupC), winPropsObj);
        settings->setValue("shouldStoreGeometry", shouldStoreGeometry);
        
        settings->endGroup();
    }

    void WindowSystemPrivate::windowCreated(WindowInterface *windowInterface) {
        Q_Q(WindowSystem);
        windowMap.insert(windowInterface->window(), windowInterface);
        windowInterfaces.append(windowInterface, 0);
        Q_EMIT q->windowCreated(windowInterface);
    }

    void WindowSystemPrivate::windowAboutToDestroy(WindowInterface *windowInterface) {
        Q_Q(WindowSystem);
        windowMap.remove(windowInterface->window());
        windowInterfaces.remove(windowInterface);
        Q_EMIT q->windowAboutToDestroy(windowInterface);
    }

    static WindowSystem *m_instance = nullptr;

    WindowSystem::WindowSystem(QObject *parent) : WindowSystem(*new WindowSystemPrivate(), parent) {
    }

    WindowSystem::~WindowSystem() {
        Q_D(WindowSystem);

        d->saveSettings();

        m_instance = nullptr;
    }

    WindowInterface *WindowSystem::findWindow(QWindow *window) const {
        Q_D(const WindowSystem);
        if (!window)
            return nullptr;
        return d->windowMap.value(window, nullptr);
    }

    int WindowSystem::count() const {
        Q_D(const WindowSystem);
        return d->windowInterfaces.size();
    }

    QList<WindowInterface *> WindowSystem::windows() const {
        Q_D(const WindowSystem);
        return d->windowInterfaces.keys_qlist();
    }

    WindowInterface *WindowSystem::firstWindow() const {
        Q_D(const WindowSystem);
        return d->windowInterfaces.empty() ? nullptr : d->windowInterfaces.begin()->first;
    }

    using WindowSizeTrimmers = QHash<QString, WindowSizeTrimmer *>;

    Q_GLOBAL_STATIC(WindowSizeTrimmers, winSizeTrimmers)

    class WindowSizeTrimmer : public QObject {
    public:
        WindowSizeTrimmer(const QString &id, QWindow *w) : QObject(w), id(id) {
            winSizeTrimmers->insert(id, this);

            window = w;
            m_pos = w->position();
            m_obsolete = false;
            w->installEventFilter(this);
        }

        ~WindowSizeTrimmer() {
            winSizeTrimmers->remove(id);
        };

        QPoint pos() const {
            return m_pos;
        }

        void makeObsolete() {
            m_obsolete = true;
            deleteLater();
        }

        bool obsolete() const {
            return m_obsolete;
        }

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override {
            switch (event->type()) {
                case QEvent::Move:
                    if (window && window->isVisible()) {
                        makeObsolete();
                    }
                    break;
                default:
                    break;
            }
            return QObject::eventFilter(obj, event);
        }

        QPointer<QWindow> window;
        QString id;
        QPoint m_pos;
        bool m_obsolete;
    };

    static void centralizeWindow(QWindow *w, QSizeF ratio = QSizeF(-1, -1)) {
        QSize desktopSize;
        if (w->transientParent()) {
            desktopSize = static_cast<QWindow*>(w->transientParent())->size();
        } else {
            desktopSize = w->screen()->size();
        }

        int dw = desktopSize.width();
        int dh = desktopSize.height();

        double rw = ratio.width();
        double rh = ratio.height();

        QSize size = w->size();
        if (rw > 0 && rw <= 1) {
            size.setWidth(dw * rw);
        }
        if (rh > 0 && rh <= 1) {
            size.setHeight(dh * rh);
        }

        w->setGeometry((dw - size.width()) / 2, (dh - size.height()) / 2, size.width(),
                       size.height());
    }

    void WindowSystem::loadGeometry(const QString &id, QWindow *w, const QSize &fallback) const {
        Q_D(const WindowSystem);
        if (!d->shouldStoreGeometry)
            return;

        qCInfo(lcWindowSystem) << "Loading window geometry for" << id << w << fallback;

        auto winProp = d->winGeometries.value(id, {});

        const auto &winRect = winProp.geometry;
        const auto &isMax = winProp.maximized;

        bool isDialog = w->transientParent() && (w->flags() & Qt::Dialog);
        qCDebug(lcWindowSystem) << "Window is dialog:" << isDialog;
        if (winRect.size().isEmpty() || isMax) {
            qCDebug(lcWindowSystem) << "Saved window geometry is empty or maximized:" << isMax;
            // Adjust sizes
            w->resize(fallback.isValid() ? fallback
                                         : (QApplication::primaryScreen()->size() * 0.75));
            if (!isDialog) {
                centralizeWindow(w);
            }
            if (isMax) {
                w->showMaximized();
            }
        } else {
            qCDebug(lcWindowSystem) << "Saved window geometry is valid:" << winRect;
            if (isDialog)
                w->resize(winRect.size());
            else
                w->setGeometry(winRect);
        }

        double ratio = (w->screen()->devicePixelRatio());
        auto addTrimmer = [&](bool move) {
            if (!isDialog && !isMax) {
                qCDebug(lcWindowSystem) << "Trimmer" << move;
                if (move) {
                    QRect rect = w->geometry();
                    w->setGeometry(QRect(rect.topLeft() + QPoint(30, 30) * ratio, rect.size()));
                }

                new WindowSizeTrimmer(id, w);
            }
        };

        // Check trimmers
        auto trimmer = winSizeTrimmers->value(id, {});
        if (trimmer && !trimmer->obsolete()) {
            trimmer->makeObsolete();
            addTrimmer(true);
        } else {
            addTrimmer(false);
        }
    }

    void WindowSystem::saveGeometry(const QString &id, QWindow *w) {
        Q_D(WindowSystem);
        if (!d->shouldStoreGeometry)
            return;
        qCInfo(lcWindowSystem) << "Saving window geometry for" << id << w << w->geometry() << w->visibility();
        d->winGeometries.insert(id, {w->geometry(), w->visibility() == QWindow::Maximized});
    }

    bool WindowSystem::shouldStoreGeometry() const {
        Q_D(const WindowSystem);
        return d->shouldStoreGeometry;
    }

    void WindowSystem::setShouldStoreGeometry(bool on) {
        Q_D(WindowSystem);
        if (d->shouldStoreGeometry != on) {
            d->shouldStoreGeometry = on;
            Q_EMIT shouldStoreGeometryChanged(on);
        }
    }

    WindowSystem::WindowSystem(WindowSystemPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        m_instance = this;

        d.q_ptr = this;
        d.init();
    }
    WindowInterface *WindowSystem::firstWindowOfTypeImpl(const QMetaObject &type) const {
        Q_D(const WindowSystem);
        auto it = std::ranges::find_if(d->windowInterfaces, [=](const auto &p) -> bool {
            return p.first->inherits(type.className());
        });
        if (it == d->windowInterfaces.end()) {
            return nullptr;
        }
        return it.key();
    }
}

#include "moc_windowsystem.cpp"
