#include "windowsystem.h"
#include "windowsystem_p.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QPointer>
#include <QScreen>
#include <QSplitter>

#include <QMCore/qmbatch.h>
#include <QMCore/qmsystem.h>
#include <QMWidgets/qmview.h>

#include "iloader.h"

namespace Core {

#define myWarning(func) (qWarning().nospace() << "Core::WindowSystem::" << (func) << "():").space()

    static const char settingCatalogC[] = "WindowSystem";

    static const char winGeometryGroupC[] = "WindowGeometry";

    static const char splitterSizesGroupC[] = "SplitterSizes";

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

    SplitterSizes SplitterSizes::fromObject(const QJsonObject &obj) {
        return QM::strListToIntList(QM::jsonArrayToStrList(obj.value("sizes").toArray()));
    }

    template <class V, template <class> class Array, class T, class Mapper>
    static Array<V> Select(const Array<T> &list, Mapper mapper) { // Same collection
        Array<V> res;
        res.reserve(list.size());
        for (const auto &item : qAsConst(list)) {
            res.push_back(mapper(item));
        }
        return res;
    }

    QJsonObject SplitterSizes::toObject() const {
        return {
            {"sizes", QJsonArray::fromStringList(Select<QString>(
                          sizes, [](int num) -> QString { return QString::number(num); }))}
        };
    }

    WindowSystemPrivate::WindowSystemPrivate() : q_ptr(nullptr) {
    }

    WindowSystemPrivate::~WindowSystemPrivate() {
    }

    void WindowSystemPrivate::init() {
        readSettings();
    }

    void WindowSystemPrivate::readSettings() {
        winGeometries.clear();

        auto settings = ILoader::instance()->settings();
        auto obj = settings->value(settingCatalogC).toObject();

        auto winPropsObj = obj.value(winGeometryGroupC).toObject();
        for (auto it = winPropsObj.begin(); it != winPropsObj.end(); ++it) {
            if (!it->isObject()) {
                continue;
            }
            winGeometries.insert(it.key(), WindowGeometry::fromObject(it->toObject()));
        }

        auto spPropsObj = obj.value(splitterSizesGroupC).toObject();
        for (auto it = spPropsObj.begin(); it != spPropsObj.end(); ++it) {
            if (!it->isObject()) {
                continue;
            }
            splitterSizes.insert(it.key(), SplitterSizes::fromObject(it->toObject()));
        }
    }

    void WindowSystemPrivate::saveSettings() const {
        auto settings = ILoader::instance()->settings();

        QJsonObject winPropsObj;
        for (auto it = winGeometries.begin(); it != winGeometries.end(); ++it) {
            winPropsObj.insert(it.key(), it->toObject());
        }

        QJsonObject spPropsObj;
        for (auto it = splitterSizes.begin(); it != splitterSizes.end(); ++it) {
            spPropsObj.insert(it.key(), it->toObject());
        }

        QJsonObject obj;
        obj.insert(winGeometryGroupC, winPropsObj);
        obj.insert(splitterSizesGroupC, spPropsObj);

        settings->insert(settingCatalogC, obj);
    }

    void WindowSystemPrivate::windowCreated(IWindow *iWin) {
        Q_Q(WindowSystem);
        windowMap.insert(iWin->window(), iWin);
        iWindows.append(iWin);
    }

    void WindowSystemPrivate::windowAboutToDestroy(IWindow *iWin) {
        windowMap.remove(iWin->window());
        iWindows.remove(iWin);
    }

    static WindowSystem *m_instance = nullptr;

    WindowSystem::WindowSystem(QObject *parent) : WindowSystem(*new WindowSystemPrivate(), parent) {
    }

    WindowSystem::~WindowSystem() {
        Q_D(WindowSystem);

        d->saveSettings();

        m_instance = nullptr;
    }

    IWindow *WindowSystem::findWindow(QWidget *window) const {
        Q_D(const WindowSystem);
        if (!window)
            return nullptr;
        window = window->window();
        return d->windowMap.value(window, nullptr);
    }

    int WindowSystem::count() const {
        Q_D(const WindowSystem);
        return d->iWindows.size();
    }

    QList<IWindow *> WindowSystem::windows() const {
        Q_D(const WindowSystem);
        return d->iWindows.values_qlist();
    }

    IWindow *WindowSystem::firstWindow() const {
        Q_D(const WindowSystem);
        return d->iWindows.isEmpty() ? nullptr : *d->iWindows.begin();
    }

    using WindowSizeTrimmers = QHash<QString, WindowSizeTrimmer *>;

    Q_GLOBAL_STATIC(WindowSizeTrimmers, winSizeTrimmers)

    class WindowSizeTrimmer : public QObject {
    public:
        WindowSizeTrimmer(const QString &id, QWidget *w) : QObject(w), id(id) {
            winSizeTrimmers->insert(id, this);

            widget = w;
            m_pos = w->pos();
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
                    // case QEvent ::WindowStateChange: {
                    //     auto e = static_cast<QWindowStateChangeEvent *>(event);
                    //     if ((e->oldState() & Qt::WindowMaximized) || !(widget->windowState() &
                    //     Qt::WindowMaximized))
                    //     {
                    //         break;
                    //     }
                    // }
                case QEvent::Move:
                    if (widget->isVisible()) {
                        makeObsolete();
                    }
                    break;
                default:
                    break;
            }
            return QObject::eventFilter(obj, event);
        }

        QPointer<QWidget> widget;
        QString id;
        QPoint m_pos;
        bool m_obsolete;
    };

    void WindowSystem::loadGeometry(const QString &id, QWidget *w,
                                          const QSize &fallback) const {
        Q_D(const WindowSystem);

        auto winProp = d->winGeometries.value(id, {});

        const auto &winRect = winProp.geometry;
        const auto &isMax = winProp.maximized;

        bool isDialog = w->parentWidget() && (w->windowFlags() & Qt::Dialog);
        if (winRect.size().isEmpty() || isMax) {
            // Adjust sizes
            w->resize(fallback.isValid() ? fallback : (QApplication::primaryScreen()->size() * 0.75));
            if (!isDialog) {
                QMView::centralizeWindow(w);
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

        double ratio = (w->screen()->logicalDotsPerInch() / QM::unitDpi());
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

    void WindowSystem::saveGeometry(const QString &id, QWidget *w) {
        Q_D(WindowSystem);
        d->winGeometries.insert(id, {w->geometry(), w->isMaximized()});
    }

    void WindowSystem::loadSplitterSizes(const QString &id, QSplitter *s,
                                         const QList<int> &fallback) const {
        Q_D(const WindowSystem);

        auto spProp = d->splitterSizes.value(id, {});
        if (spProp.sizes.size() != s->count()) {
            if (fallback.size() == s->count())
                s->setSizes(fallback);
        } else {
            s->setSizes(spProp.sizes);
        }
    }

    void WindowSystem::saveSplitterSizes(const QString &id, QSplitter *s) {
        Q_D(WindowSystem);
        d->splitterSizes.insert(id, {s->sizes()});
    }

    WindowSystem::WindowSystem(WindowSystemPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        m_instance = this;

        d.q_ptr = this;
        d.init();
    }
}
