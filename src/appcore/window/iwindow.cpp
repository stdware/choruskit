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
        QMChronoSet<QKeySequence> duplicatedKeys;
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
            myWarning(__func__) << "duplicated shortcuts detected" << action
                                << duplicatedKeys.values_qlist();
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
                case QEvent::DragEnter:
                case QEvent::Drop: {
                    auto e = static_cast<QDropEvent *>(event);
                    const QMimeData *mime = e->mimeData();
                    if (mime->hasUrls()) {
                        QFileInfoList dirs;
                        QHash<QString, QFileInfoList> files;
                        for (const auto &url : mime->urls()) {
                            if (url.isLocalFile()) {
                                auto info = QFileInfo(url.toLocalFile());
                                if (info.isDir()) {
                                    dirs.append(info);
                                    continue;
                                }
                                if (info.isFile()) {
                                    auto suffix = info.completeSuffix().toLower();
                                    files[suffix].append(info);
                                    continue;
                                }
                            }
                        }

                        if (event->type() == QEvent::DragEnter) {
                            if ((!dirs.isEmpty() &&
                                 d->dragFileHandlerMap.contains(QStringLiteral("/"))) ||
                                (!files.isEmpty() &&
                                 d->dragFileHandlerMap.contains(QStringLiteral("*")))) {
                                e->acceptProposedAction();
                            } else {
                                for (auto it = files.begin(); it != files.end(); ++it) {
                                    auto it2 = d->dragFileHandlerMap.find(it.key());
                                    if (it2 == d->dragFileHandlerMap.end()) {
                                        continue;
                                    }
                                    if (it2->max == 0 || it2->max >= it->size()) {
                                        e->acceptProposedAction();
                                        break;
                                    }
                                }
                            }
                        } else {
                            bool accept = false;

                            // Handle directories
                            if (!dirs.isEmpty()) {
                                auto it = d->dragFileHandlerMap.find(QStringLiteral("/"));
                                if (it != d->dragFileHandlerMap.end()) {
                                    for (const auto &dir : qAsConst(dirs)) {
                                        it->func(dir.absoluteFilePath());
                                    }
                                    accept = true;
                                }
                            }

                            // Handle files
                            for (auto it = files.begin(); it != files.end();) {
                                auto it2 = d->dragFileHandlerMap.find(it.key());
                                if (it2 != d->dragFileHandlerMap.end()) {
                                    const auto &handler = *it2;
                                    if (handler.max == 0 || handler.max >= it->size()) {
                                        for (const auto &file : qAsConst(it.value())) {
                                            handler.func(file.absoluteFilePath());
                                        }
                                        accept = true;
                                    }
                                    it = files.erase(it);
                                    continue;
                                }
                                ++it;
                            }

                            // Handle unhandled files
                            if (!files.isEmpty()) {
                                auto it = d->dragFileHandlerMap.find(QStringLiteral("*"));
                                if (it != d->dragFileHandlerMap.end()) {
                                    for (const auto &fileList : qAsConst(files)) {
                                        for (const auto &file : fileList) {
                                            it->func(file.absoluteFilePath());
                                        }
                                    }
                                    accept = true;
                                }
                            }

                            if (accept) {
                                e->acceptProposedAction();
                            }
                        }
                    }

                    if (!e->isAccepted()) {
                        return true;
                    }
                    break;
                }

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

        auto winMgr = ICoreBase::instance()->windowSystem();
        auto d = winMgr->d_func();
        d->iWindows.append(q);

        // Get quit control
        // qApp->setQuitOnLastWindowClosed(false);

        // Create window
        auto win = q->createWindow(nullptr);

        // Add to indexes
        d->windowMap.insert(win, q);

        win->setAttribute(Qt::WA_DeleteOnClose);
        connect(qApp, &QApplication::aboutToQuit, win,
                &QWidget::close); // Ensure closing window when quit

        q->setWindow(win);
        shortcutCtx = new QMShortcutContext(this);
        winFilter = new WindowEventFilter(this, win);

        q->addShortcutContext(win, IWindow::Stable); // Default shortcut context

        IExecutivePrivate::load(enableDelayed);

        Q_EMIT winMgr->windowCreated(q);

        win->show();
    }

    void IWindowPrivate::quit() {
        Q_Q(IWindow);

        stopDelayedTimer();

        auto w = q->window();
        if (!w->isHidden())
            w->hide();

        changeLoadState(IExecutive::Exiting);

        ICoreBase::instance()->windowSystem()->d_func()->windowExit(q);

        // Delete addOns
        for (auto it2 = addOns.rbegin(); it2 != addOns.rend(); ++it2) {
            auto &addOn = *it2;
            delete addOn;
        }

        delete shortcutCtx;
        shortcutCtx = nullptr;

        changeLoadState(IExecutive::Deleted);

        q->setWindow(nullptr);
    }

    bool IWindow::closeAsExit() const {
        Q_D(const IWindow);
        return d->closeAsExit;
    }

    void IWindow::setCloseAsExit(bool on) {
        Q_D(IWindow);
        d->closeAsExit = on;
    }

    QString IWindow::id() const {
        Q_D(const IWindow);
        return d->id;
    }

    void IWindow::addWidget(const QString &id, QWidget *w) {
        Q_D(IWindow);
        if (!w) {
            myWarning(__func__) << "trying to add null widget";
            return;
        }
        if (d->widgetMap.contains(id)) {
            myWarning(__func__) << "trying to add duplicated widget:" << id;
            return;
        }
        d->widgetMap.insert(id, w);
        Q_EMIT widgetAdded(id, w);
    }

    void IWindow::removeWidget(const QString &id) {
        Q_D(IWindow);
        auto it = d->widgetMap.find(id);
        if (it == d->widgetMap.end()) {
            myWarning(__func__) << "action item does not exist:" << id;
            return;
        }
        auto w = it.value();
        Q_EMIT aboutToRemoveWidget(id, w);
        d->widgetMap.erase(it);
    }

    QWidget *IWindow::widget(const QString &id) const {
        Q_D(const IWindow);
        auto it = d->widgetMap.find(id);
        if (it != d->widgetMap.end()) {
            return it.value();
        }
        return nullptr;
    }

    QWidgetList IWindow::widgets() const {
        Q_D(const IWindow);
        return d->widgetMap.values();
    }

    void IWindow::addActionItem(ActionItem *item) {
        Q_D(IWindow);
        if (!item) {
            myWarning(__func__) << "trying to add null action item";
            return;
        }

        if (d->actionItemMap.contains(item->id())) {
            myWarning(__func__) << "trying to add duplicated action item:" << item->id();
            return;
        }
        d->actionItemMap.append(item->id(), item);

        switch (item->type()) {
            case ActionItem::Action: {
                window()->addAction(item->action());
                break;
            }
            case ActionItem::Standalone: {
                addShortcutContext(item->standalone(), Stable);
                break;
            }
            default:
                break;
        }
    }

    void IWindow::removeActionItem(Core::ActionItem *item) {
        if (item == nullptr) {
            myWarning(__func__) << "trying to remove null item";
            return;
        }
        removeActionItem(item->id());
    }

    void IWindow::removeActionItem(const QString &id) {
        Q_D(IWindow);
        auto it = d->actionItemMap.find(id);
        if (it == d->actionItemMap.end()) {
            myWarning(__func__) << "action item does not exist:" << id;
            return;
        }
        auto item = it.value();
        d->actionItemMap.erase(it);

        switch (item->type()) {
            case ActionItem::Action: {
                window()->removeAction(item->action());
                break;
            }
            case ActionItem::Standalone: {
                removeShortcutContext(item->standalone());
                break;
            }
            default:
                break;
        }
    }

    ActionItem *IWindow::actionItem(const QString &id) const {
        Q_D(const IWindow);
        return d->actionItemMap.value(id, nullptr);
    }

    QList<ActionItem *> IWindow::actionItems() const {
        Q_D(const IWindow);
        return d->actionItemMap.values_qlist();
    }

    void IWindow::addShortcutContext(QWidget *w, ShortcutContextPriority priority) {
        Q_D(IWindow);
        d->shortcutCtx->addWidget(w, priority);
    }

    void IWindow::removeShortcutContext(QWidget *w) {
        Q_D(IWindow);
        d->shortcutCtx->removeWidget(w);
    }

    QList<QWidget *> IWindow::shortcutContexts() const {
        Q_D(const IWindow);
        return d->shortcutCtx->widgets();
    }

    bool IWindow::hasDragFileHandler(const QString &suffix) {
        Q_D(const IWindow);
        if (suffix.isEmpty())
            return false;

        return d->dragFileHandlerMap.contains(suffix.toLower());
    }

    void IWindow::setDragFileHandler(const QString &suffix,
                                     const std::function<void(const QString &)> &handler,
                                     int maxCount) {
        Q_D(IWindow);

        if (suffix.isEmpty())
            return;

        if (maxCount < 0) {
            removeDragFileHandler(suffix);
            return;
        }
        d->dragFileHandlerMap[suffix.toLower()] = {handler, maxCount};
    }

    void IWindow::removeDragFileHandler(const QString &suffix) {
        Q_D(IWindow);
        if (suffix.isEmpty())
            return;

        d->dragFileHandlerMap.remove(suffix.toLower());
    }

    IWindow::IWindow(const QString &id, QObject *parent)
        : IWindow(*new IWindowPrivate(), id, parent) {
    }

    IWindow::~IWindow() {
    }

    IWindow::IWindow(IWindowPrivate &d, const QString &id, QObject *parent)
        : IExecutive(d, parent) {
        d.q_ptr = this;
        d.id = id;

        d.init();
    }

}
