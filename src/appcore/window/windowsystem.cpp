#include "windowsystem.h"
#include "windowsystem_p.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QPointer>
#include <QScreen>
#include <QWindow>
#include <QJsonArray>
#include <QJsonObject>

#include "plugindatabase.h"

namespace Core {

#define myWarning(func) (qWarning().nospace() << "Core::WindowSystem::" << (func) << "():").space()

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

        auto settings = PluginDatabase::settings();
        settings->beginGroup(QLatin1String(settingCatalogC));

        auto winPropsObj = settings->value(winGeometryGroupC).toJsonObject();
        for (auto it = winPropsObj.begin(); it != winPropsObj.end(); ++it) {
            if (!it->isObject()) {
                continue;
            }
            winGeometries.insert(it.key(), WindowGeometry::fromObject(it->toObject()));
        }

        settings->endGroup();
    }

    void WindowSystemPrivate::saveSettings() const {
        QJsonObject winPropsObj;
        for (auto it = winGeometries.begin(); it != winGeometries.end(); ++it) {
            winPropsObj.insert(it.key(), it->toObject());
        }

        auto settings = PluginDatabase::settings();
        settings->beginGroup(QLatin1String(settingCatalogC));

        settings->setValue(QLatin1String(winGeometryGroupC), winPropsObj);
        
        settings->endGroup();
    }

    void WindowSystemPrivate::windowCreated(IWindow *iWin) {
        Q_Q(WindowSystem);
        windowMap.insert(iWin->window(), iWin);
        iWindows.append(iWin, 0);
        Q_EMIT q->windowCreated(iWin);
    }

    void WindowSystemPrivate::windowAboutToDestroy(IWindow *iWin) {
        Q_Q(WindowSystem);
        windowMap.remove(iWin->window());
        iWindows.remove(iWin);
        Q_EMIT q->windowAboutToDestroy(iWin);
    }

    static WindowSystem *m_instance = nullptr;

    WindowSystem::WindowSystem(QObject *parent) : WindowSystem(*new WindowSystemPrivate(), parent) {
    }

    WindowSystem::~WindowSystem() {
        Q_D(WindowSystem);

        d->saveSettings();

        m_instance = nullptr;
    }

    IWindow *WindowSystem::findWindow(QWindow *window) const {
        Q_D(const WindowSystem);
        if (!window)
            return nullptr;
        return d->windowMap.value(window, nullptr);
    }

    int WindowSystem::count() const {
        Q_D(const WindowSystem);
        return d->iWindows.size();
    }

    QList<IWindow *> WindowSystem::windows() const {
        Q_D(const WindowSystem);
        return d->iWindows.keys_qlist();
    }

    IWindow *WindowSystem::firstWindow() const {
        Q_D(const WindowSystem);
        return d->iWindows.empty() ? nullptr : d->iWindows.begin()->first;
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

        auto winProp = d->winGeometries.value(id, {});

        const auto &winRect = winProp.geometry;
        const auto &isMax = winProp.maximized;

        bool isDialog = w->transientParent() && (w->flags() & Qt::Dialog);
        if (winRect.size().isEmpty() || isMax) {
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
            if (isDialog)
                w->resize(winRect.size());
            else
                w->setGeometry(winRect);
        }

        double ratio = (w->screen()->devicePixelRatio());
        auto addTrimmer = [&](bool move) {
            if (!isDialog && !isMax) {
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
        d->winGeometries.insert(id, {w->geometry(), w->windowState() == Qt::WindowMaximized});
    }

    WindowSystem::WindowSystem(WindowSystemPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        m_instance = this;

        d.q_ptr = this;
        d.init();
    }
}
