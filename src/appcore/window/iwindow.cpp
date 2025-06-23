#include "iwindow.h"
#include "iwindow_p.h"

#include "icorebase.h"
#include "windowsystem_p.h"

#include <QDebug>
#include <QFileInfo>
#include <QMimeData>
#include <QActionEvent>

#include <private/qwidget_p.h>

static const int DELAYED_INITIALIZE_INTERVAL = 5; // ms

QT_SPECIALIZE_STD_HASH_TO_CALL_QHASH_BY_CREF(QKeySequence)

namespace Core {

#define myWarning(func) (qWarning().nospace() << "Core::IWindow::" << (func) << "():").space()

    class ShortcutContext : public QObject {
    public:
        explicit ShortcutContext(QObject *parent = nullptr);
        ~ShortcutContext();

        void addContext(QWidget *w);
        void removeContext(QWidget *w);

        bool hasContext(QWidget *w) const;
        bool hasShortcut(const QKeySequence &key) const;

        QList<QWidget *> contexts() const {
            return shortcutContextWidgets.values();
        }

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;

    private:
        struct ActionData {
            QAction *action;
            QSet<QWidget *> widgets;
            QList<QKeySequence> keys;
        };
        QHash<QKeySequence, ActionData *> shortcutMap;
        QHash<QAction *, ActionData *> actionDataMap;
        QSet<QWidget *> shortcutContextWidgets;
        QAction *lastFlushedAction;

        void shortcutContextAdded(QWidget *w);
        void shortcutContextRemoved(QWidget *w);

        void addAction(QAction *action, QWidget *w);
        void removeAction(QAction *action, QWidget *w);
        void flushAction(QAction *action);

        void _q_shortcutContextDestroyed(QObject *obj);
    };

    ShortcutContext::ShortcutContext(QObject *parent) : QObject(parent) {
        lastFlushedAction = nullptr;
    }

    ShortcutContext::~ShortcutContext() {
        qDeleteAll(actionDataMap);
    }

    void ShortcutContext::addContext(QWidget *w) {
        if (!w) {
            myWarning(__func__) << "trying to add null widget";
            return;
        }

        if (shortcutContextWidgets.contains(w)) {
            myWarning(__func__) << "trying to add duplicated widget:" << w;
            return;
        }

        shortcutContextWidgets.insert(w);
        connect(w, &QObject::destroyed, this, &ShortcutContext::_q_shortcutContextDestroyed);

        shortcutContextAdded(w);

        w->installEventFilter(this);
    }

    void ShortcutContext::removeContext(QWidget *w) {
        auto it = shortcutContextWidgets.find(w);
        if (it == shortcutContextWidgets.end()) {
            myWarning(__func__) << "widget does not exist:" << w;
            return;
        }

        w->removeEventFilter(this);

        shortcutContextRemoved(w);

        disconnect(w, &QObject::destroyed, this, &ShortcutContext::_q_shortcutContextDestroyed);
        shortcutContextWidgets.erase(it);
    }

    bool ShortcutContext::hasContext(QWidget *w) const {
        return shortcutContextWidgets.contains(w);
    }

    bool ShortcutContext::hasShortcut(const QKeySequence &key) const {
        return shortcutMap.contains(key);
    }

    bool ShortcutContext::eventFilter(QObject *obj, QEvent *event) {
        switch (event->type()) {
            case QEvent::ActionAdded: {
                auto e = static_cast<QActionEvent *>(event);
                auto w = static_cast<QWidget *>(obj);
                addAction(e->action(), w);
                flushAction(e->action());
                break;
            }
            case QEvent::ActionChanged: {
                auto e = static_cast<QActionEvent *>(event);
                flushAction(e->action());
                break;
            }
            case QEvent::ActionRemoved: {
                auto e = static_cast<QActionEvent *>(event);
                auto w = static_cast<QWidget *>(obj);
                removeAction(e->action(), w);
                break;
            }
            default:
                break;
        }

        return QObject::eventFilter(obj, event);
    }

    void ShortcutContext::shortcutContextAdded(QWidget *w) {
        const auto &actions = w->actions();
        for (const auto &action : actions) {
            addAction(action, w);
        }
        for (const auto &action : actions) {
            flushAction(action);
        }
    }

    void ShortcutContext::shortcutContextRemoved(QWidget *w) {
        // We cannot query widgets' actions because the widget maybe destroyed
        QList<QAction *> actions;
        for (const auto &item : qAsConst(actionDataMap)) {
            if (item->widgets.contains(w)) {
                actions.append(item->action);
            }
        }

        for (const auto &action : qAsConst(actions)) {
            removeAction(action, w);
        }
    }

    void ShortcutContext::addAction(QAction *action, QWidget *w) {
        auto it = actionDataMap.find(action);
        if (it != actionDataMap.end()) {
            it.value()->widgets.insert(w);
            return;
        }

        auto data = new ActionData();
        data->action = action;
        data->widgets.insert(w);
        actionDataMap.insert(action, data);
    }

    void ShortcutContext::removeAction(QAction *action, QWidget *w) {
        auto it = actionDataMap.find(action);
        if (it == actionDataMap.end()) {
            return;
        }

        auto &set = it.value()->widgets;
        set.remove(w);
        if (!set.isEmpty()) {
            return;
        }

        for (const auto &sh : action->shortcuts()) {
            shortcutMap.remove(sh);
        }

        delete it.value();
        actionDataMap.erase(it);
    }

    void ShortcutContext::flushAction(QAction *action) {
        if (lastFlushedAction == action) {
            return;
        }

        auto data = actionDataMap.value(action);
        if (!data) {
            return;
        }

        if (!lastFlushedAction) {
            // Reset this pointer after all actions has been handled
            QTimer::singleShot(0, this, [this]() {
                lastFlushedAction = nullptr; //
            });
        }
        lastFlushedAction = action;

        // Remove old keys
        for (const auto &key : qAsConst(data->keys)) {
            shortcutMap.remove(key);
        }

        QList<QKeySequence> keys;
        QList<QKeySequence> duplicatedKeys;
        for (const auto &sh : action->shortcuts()) {
            if (sh.isEmpty())
                continue;
            if (shortcutMap.contains(sh)) {
                duplicatedKeys.append(sh);
                continue;
            }
            keys.append(sh);
            shortcutMap.insert(sh, data);
        }

        // Avoid recursive signal handling
        action->blockSignals(true);
        action->setShortcuts(keys);
        action->blockSignals(false);

        data->keys = keys;

        if (!duplicatedKeys.isEmpty()) {
            myWarning(__func__) << "duplicated shortcuts detected" << action << duplicatedKeys;
        }
    }

    void ShortcutContext::_q_shortcutContextDestroyed(QObject *obj) {
        auto w = static_cast<QWidget *>(obj);
        shortcutContextRemoved(w);
        shortcutContextWidgets.remove(w);
    }

    class WindowEventFilter : public QObject {
    public:
        explicit WindowEventFilter(IWindowPrivate *d, QWidget *w);
        ~WindowEventFilter();

        IWindowPrivate *d;
        QWidget *w;

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;
    };

    WindowEventFilter::WindowEventFilter(IWindowPrivate *d, QWidget *w) : QObject(d), d(d), w(w) {
        w->installEventFilter(this);
    }

    WindowEventFilter::~WindowEventFilter() {
    }

    bool WindowEventFilter::eventFilter(QObject *obj, QEvent *event) {
        if (obj == w) {
            switch (event->type()) {
                case QEvent::Close:
                    if (d->closeAsExit && event->isAccepted()) {
                        d->quit();
                    }
                    break;
                default:
                    break;
            }
        }
        return QObject::eventFilter(obj, event);
    }

    class QWidgetHacker : public QWidget {
    public:
        int actionCount() const {
            auto d = static_cast<QWidgetPrivate *>(d_ptr.data());
            return d->actions.count();
        }

        friend class IWindow;
        friend class IWindowPrivate;
    };



    IWindowAddOnPrivate::IWindowAddOnPrivate() {
    }

    IWindowAddOnPrivate::~IWindowAddOnPrivate() = default;

    void IWindowAddOnPrivate::init() {
    }

    IWindowAddOn::IWindowAddOn(QObject *parent) : IWindowAddOn(*new IWindowAddOnPrivate(), parent) {
    }

    IWindowAddOn::~IWindowAddOn() {
    }

    IWindow *IWindowAddOn::windowHandle() const {
        Q_D(const IWindowAddOn);
        return static_cast<IWindow *>(d->host);
    }

    IWindowAddOn::IWindowAddOn(IWindowAddOnPrivate &d, QObject *parent)
        : IExecutiveAddOn(*new IWindowAddOnPrivate(), parent) {
        d.init();
    }

    IWindowPrivate::IWindowPrivate() {
        closeAsExit = true;
    }

    IWindowPrivate::~IWindowPrivate() {
    }

    void IWindowPrivate::init() {
    }

    void IWindowPrivate::load(bool enableDelayed) {
        Q_Q(IWindow);

        // Create window
        auto win = q->createWindow(nullptr);
        win->setAttribute(Qt::WA_DeleteOnClose);

        // Ensure closing window when quit
        connect(qApp, &QApplication::aboutToQuit, win, &QWidget::close);

        q->setWindow(win);
        winFilter = new WindowEventFilter(this, win);

        IExecutivePrivate::load(enableDelayed);

        ICoreBase::instance()->windowSystem()->d_func()->windowCreated(q);

        win->show();
    }

    void IWindowPrivate::quit() {
        Q_Q(IWindow);

        auto w = q->window();
        if (!w->isHidden())
            w->hide();

        ICoreBase::instance()->windowSystem()->d_func()->windowAboutToDestroy(q);

        IExecutivePrivate::quit();

        q->setWindow(nullptr);
        q->deleteLater();
    }

    IWindow::~IWindow() {
    }

    bool IWindow::closeAsExit() const {
        Q_D(const IWindow);
        return d->closeAsExit;
    }

    void IWindow::setCloseAsExit(bool on) {
        Q_D(IWindow);
        d->closeAsExit = on;
    }

    IWindow::IWindow(QObject *parent) : IWindow(*new IWindowPrivate(), parent) {
    }

    IWindow::IWindow(IWindowPrivate &d, QObject *parent) : IExecutive(d, parent) {
        d.init();
    }

}
