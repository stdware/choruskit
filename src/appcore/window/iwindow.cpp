#include "iwindow.h"
#include "iwindow_p.h"

#include "icorebase.h"
#include "iwindowaddon_p.h"
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
                auto w = qobject_cast<QWidget *>(obj);
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
                auto w = qobject_cast<QWidget *>(obj);
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
                                << duplicatedKeys.values();
        }
    }

    void ShortcutContext::_q_shortcutContextDestroyed(QObject *obj) {
        auto w = static_cast<QWidget *>(obj);
        shortcutContextRemoved(w);
        shortcutContextWidgets.remove(w);
    }

    class WindowCloseFilter : public QObject {
    public:
        explicit WindowCloseFilter(IWindowPrivate *d, QWidget *w);
        ~WindowCloseFilter();

        IWindowPrivate *d;
        QWidget *w;

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;
    };

    static void qtimer_single_shot_impl(QObject *obj, const char *member, const QString &s) {
        QMetaObject::invokeMethod(obj, member, Qt::DirectConnection, Q_ARG(QString, s));
    }

    WindowCloseFilter::WindowCloseFilter(IWindowPrivate *d, QWidget *w) : QObject(d), d(d), w(w) {
        w->installEventFilter(this);
    }

    WindowCloseFilter::~WindowCloseFilter() {
    }

    bool WindowCloseFilter::eventFilter(QObject *obj, QEvent *event) {
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
                            if ((!dirs.isEmpty() && d->dragFileHandlerMap.contains("/")) ||
                                (!files.isEmpty() && d->dragFileHandlerMap.contains("*"))) {
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
                                auto it = d->dragFileHandlerMap.find("/");
                                if (it != d->dragFileHandlerMap.end()) {
                                    for (const auto &dir : qAsConst(dirs)) {
                                        qtimer_single_shot_impl(it->obj, it->member,
                                                                dir.absoluteFilePath());
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
                                            qtimer_single_shot_impl(handler.obj, handler.member,
                                                                    file.absoluteFilePath());
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
                                auto it = d->dragFileHandlerMap.find("*");
                                if (it != d->dragFileHandlerMap.end()) {
                                    for (const auto &fileList : qAsConst(files)) {
                                        for (const auto &file : fileList) {
                                            qtimer_single_shot_impl(it->obj, it->member,
                                                                    file.absoluteFilePath());
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
                    if (event->isAccepted()) {
                        d->windowClosed(w);
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

    IWindowPrivate::IWindowPrivate() {
    }

    IWindowPrivate::~IWindowPrivate() {
        tryStopDelayedTimer();
    }

    void IWindowPrivate::init() {
    }

    void IWindowPrivate::changeLoadState(IWindow::State state) {
        Q_Q(IWindow);
        q->nextLoadingState(state);
        this->state = state;
        Q_EMIT q->loadingStateChanged(state);
    }

    void IWindowPrivate::setWindow(QWidget *w, WindowSystemPrivate *d) {
        Q_Q(IWindow);

        q->setWindow(w);

        shortcutCtx = new QMShortcutContext(this);

        closeFilter = new WindowCloseFilter(this, q->window());

        // Setup window
        changeLoadState(IWindow::WindowSetup);

        // Call all add-ons
        auto facs = d->addOnFactories.value(id);
        for (auto it = facs.begin(); it != facs.end(); ++it) {
            const auto &mo = it.key();
            const auto &fac = it.value();

            IWindowAddOn *addOn;
            if (fac) {
                addOn = fac();
            } else {
                addOn = qobject_cast<IWindowAddOn *>(mo->newInstance());
            }

            if (!addOn) {
                myWarning(__func__)
                    << "window add-on factory creates null instance:" << mo->className();
                continue;
            }

            addOn->d_ptr->iWin = q;
            addOns.push_back(addOn);
        }

        // Initialize
        for (auto &addOn : qAsConst(addOns)) {
            // Call 1
            addOn->initialize();
        }

        changeLoadState(IWindow::Initialized);

        // ExtensionsInitialized
        for (auto it2 = addOns.rbegin(); it2 != addOns.rend(); ++it2) {
            auto &addOn = *it2;
            // Call 2
            addOn->extensionsInitialized();
        }

        // Add-ons finished
        changeLoadState(IWindow::Running);

        // Delayed initialize
        delayedInitializeQueue = addOns;

        delayedInitializeTimer = new QTimer();
        delayedInitializeTimer->setInterval(DELAYED_INITIALIZE_INTERVAL);
        delayedInitializeTimer->setSingleShot(true);
        connect(delayedInitializeTimer, &QTimer::timeout, this,
                &IWindowPrivate::nextDelayedInitialize);
        delayedInitializeTimer->start();
    }

    void IWindowPrivate::deleteAllAddOns() {
        for (auto it2 = addOns.rbegin(); it2 != addOns.rend(); ++it2) {
            auto &addOn = *it2;
            // Call 1
            delete addOn;
        }
    }

    void IWindowPrivate::tryStopDelayedTimer() {
        // Stop delayed initializations
        if (delayedInitializeTimer) {
            if (delayedInitializeTimer->isActive()) {
                delayedInitializeTimer->stop();
            }
            delete delayedInitializeTimer;
            delayedInitializeTimer = nullptr;
        }
    }

    void IWindowPrivate::nextDelayedInitialize() {
        Q_Q(IWindow);

        while (!delayedInitializeQueue.empty()) {
            auto addOn = delayedInitializeQueue.front();
            delayedInitializeQueue.pop_front();

            bool delay = addOn->delayedInitialize();
            if (delay)
                break; // do next delayedInitialize after a delay
        }
        if (delayedInitializeQueue.empty()) {
            delete delayedInitializeTimer;
            delayedInitializeTimer = nullptr;
            Q_EMIT q->initializationDone();
        } else {
            delayedInitializeTimer->start();
        }
    }

    void IWindowPrivate::windowClosed(QWidget *w) {
        Q_Q(IWindow);

        Q_UNUSED(w);

        tryStopDelayedTimer();

        if (!w->isHidden())
            w->hide();

        changeLoadState(IWindow::Closed);

        ICoreBase::instance()->windowSystem()->d_func()->windowClosed(q);

        delete shortcutCtx;
        shortcutCtx = nullptr;

        // Delete addOns
        deleteAllAddOns();

        changeLoadState(IWindow::Deleted);

        q->setWindow(nullptr);
        delete q;
    }

    void IWindow::load() {
        auto winMgr = ICoreBase::instance()->windowSystem();
        auto d = winMgr->d_func();
        d->iWindows.append(this);

        // Get quit control
        // qApp->setQuitOnLastWindowClosed(false);

        // Create window
        auto win = createWindow(nullptr);

        // Add to indexes
        d->windowMap.insert(win, this);

        win->setAttribute(Qt::WA_DeleteOnClose);
        connect(qApp, &QApplication::aboutToQuit, win,
                &QWidget::close); // Ensure closing window when quit

        d_func()->setWindow(win, d);

        Q_EMIT winMgr->windowCreated(this);

        win->show();
    }

    IWindow::State IWindow::state() const {
        Q_D(const IWindow);
        return d->state;
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
        if (d->actionItemMap.contains(id)) {
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

        if (!item->spec()) {
            myWarning(__func__) << "trying to add unidentified item" << item->id();
            return;
        }

        if (d->actionItemMap.contains(item->id())) {
            myWarning(__func__) << "trying to add duplicated action item:" << item->id();
            return;
        }
        d->actionItemMap.append(item->id(), item);

        actionItemAdded(item);
    }

    void IWindow::addActionItems(const QList<Core::ActionItem *> &items) {
        for (const auto &item : items) {
            addActionItem(item);
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

        actionItemRemoved(item);
    }

    ActionItem *IWindow::actionItem(const QString &id) const {
        Q_D(const IWindow);
        return d->actionItemMap.value(id, nullptr);
    }

    QList<ActionItem *> IWindow::actionItems() const {
        Q_D(const IWindow);
        const auto &arr = d->actionItemMap.values();
        return {arr.begin(), arr.end()};
    }

    void IWindow::addTopLevelMenu(const QString &id, QWidget *w) {
        Q_D(IWindow);
        if (!w) {
            myWarning(__func__) << "trying to add null widget";
            return;
        }

        if (d->topLevelMenuMap.contains(id)) {
            myWarning(__func__) << "trying to add duplicated widget:" << id;
            return;
        }
        d->topLevelMenuMap.insert(id, w);
        topLevelMenuAdded(id, w);
    }

    void IWindow::removeTopLevelMenu(const QString &id) {
        Q_D(IWindow);
        auto it = d->topLevelMenuMap.find(id);
        if (it == d->topLevelMenuMap.end()) {
            myWarning(__func__) << "widget does not exist:" << id;
            return;
        }
        auto w = it.value();
        d->topLevelMenuMap.erase(it);

        topLevelMenuAdded(id, w);
    }

    QWidget *IWindow::topLevelMenu(const QString &id) const {
        Q_D(const IWindow);
        return d->topLevelMenuMap.value(id, nullptr);
    }

    QMap<QString, QWidget *> IWindow::topLevelMenus() const {
        Q_D(const IWindow);
        return d->topLevelMenuMap;
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

    void IWindow::setDragFileHandler(const QString &suffix, QObject *obj, const char *member,
                                     int maxCount) {
        Q_D(IWindow);

        if (suffix.isEmpty())
            return;

        if (!obj || maxCount < 0) {
            removeDragFileHandler(suffix);
            return;
        }
        d->dragFileHandlerMap[suffix.toLower()] = {obj, member, maxCount};
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

    void IWindow::nextLoadingState(Core::IWindow::State nextState) {
        Q_UNUSED(nextState)
    }

    void IWindow::topLevelMenuAdded(const QString &id, QWidget *w) {
        // Do nothing
    }

    void IWindow::topLevelMenuRemoved(const QString &id, QWidget *w) {
        // Do nothing
    }

    void IWindow::actionItemAdded(ActionItem *item) {
        // Do nothing
    }

    void IWindow::actionItemRemoved(ActionItem *item) {
        // Do nothing
    }

    IWindow::IWindow(IWindowPrivate &d, const QString &id, QObject *parent)
        : ObjectPool(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.id = id;

        d.init();
    }

}
