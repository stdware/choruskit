#include "shortcutcontext.h"

#include <utility>

#include <QAction>
#include <QActionEvent>
#include <QEvent>
#include <QTimer>
#include <QHash>
#include <QDebug>
#include <QSet>

#include <stdcorelib/linked_map.h>

namespace Core {

    class QMShortcutContextPrivate : public QObject {
    public:
        QMShortcutContextPrivate(QMShortcutContext *q);
        ~QMShortcutContextPrivate();

        QMShortcutContext *q;

        QMap<int, stdc::linked_map<QWidget *, int /*NOT USED*/>> contexts;

        struct WidgetData {
            int priority;
            stdc::linked_map<QAction *, int /*NOT USED*/> actions;
        };

        QHash<QWidget *, WidgetData> widgets;

        struct ActionData {
            QSet<QWidget *> widgets;
            QList<QKeySequence> keys;
        };
        QHash<QAction *, ActionData> actions;

        bool actionChanged;
        bool fixing;

        void addWidget(QWidget *w, int priority);
        void removeWidget(QWidget *w);
        void postFixTask();

        void doFix();

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;

    private:
        void _q_widgetDestroyed(QObject *obj);
    };

    QMShortcutContextPrivate::QMShortcutContextPrivate(QMShortcutContext *q) : q(q) {
        actionChanged = false;
        fixing = false;
    }

    QMShortcutContextPrivate::~QMShortcutContextPrivate() {
    }

    void QMShortcutContextPrivate::addWidget(QWidget *w, int priority) {
        if (widgets.contains(w))
            return;

        contexts[priority].append(w, 0);

        const auto &actions = w->actions();

        stdc::linked_map<QAction *, int /*NOT USED*/> actionsMap;
        for (const auto &action : actions) {
            actionsMap.append(action, 0);
        }

        widgets.insert(w, {
                              priority,
                              actionsMap,
                          });

        // Add widget actions
        for (const auto &action : actions) {
            auto &data = this->actions[action];
            data.widgets.insert(w);
            data.keys = action->shortcuts(); // Add desired keys
        }

        connect(w, &QObject::destroyed, this, &QMShortcutContextPrivate::_q_widgetDestroyed);
        w->installEventFilter(this);

        postFixTask();
    }

    void QMShortcutContextPrivate::removeWidget(QWidget *w) {
        auto it = widgets.find(w);
        if (it == widgets.end())
            return;
        auto it2 = contexts.find(it->priority);
        auto &set = it2.value();
        set.remove(w);
        if (set.empty())
            contexts.erase(it2);

        // Remove actions data
        for (const auto &pair : std::as_const(it->actions)) {
            auto &a = pair.first;
            auto it = actions.find(a);
            auto &widgets = it->widgets;
            widgets.remove(w);
            if (widgets.isEmpty()) {
                fixing = true;
                a->setShortcuts(it->keys); // Restore desired keys;
                fixing = false;

                actions.erase(it);
            }
        }
        widgets.erase(it);

        w->removeEventFilter(this);
        disconnect(w, &QObject::destroyed, this, &QMShortcutContextPrivate::_q_widgetDestroyed);

        postFixTask();
    }

    void QMShortcutContextPrivate::postFixTask() {
        if (actionChanged)
            return;

        actionChanged = true;

        QTimer::singleShot(0, this, &QMShortcutContextPrivate::doFix);
    }

    void QMShortcutContextPrivate::doFix() {
        fixing = true;

        QSet<QAction *> encountered;
        QSet<QKeySequence> keys;
        for (const auto &ws : std::as_const(contexts)) {
            for (const auto &pair : ws) {
                auto &w = pair.first;
                const auto &wd = widgets[w];
                for (const auto &pair2 : wd.actions) {
                    auto &a = pair2.first;
                    if (encountered.contains(a))
                        continue;
                    encountered.insert(a);

                    const auto &ad = actions[a];
                    QList<QKeySequence> curKeys;
                    QList<QKeySequence> duplicatedKeys;
                    curKeys.reserve(ad.keys.size());
                    for (const auto &key : std::as_const(ad.keys)) {
                        if (keys.contains(key)) {
                            duplicatedKeys.append(key);
                            continue;
                        }
                        curKeys.append(key);
                        keys.insert(key);
                    }

                    if (!duplicatedKeys.isEmpty()) {
                        qDebug() << "QMShortcutContext:" << q << "duplicated shortcuts detected"
                                 << a << duplicatedKeys;
                    }

                    // Update shortcut
                    a->setShortcuts(curKeys);
                }
            }
        }

        fixing = false;
        actionChanged = false;
    }

    bool QMShortcutContextPrivate::eventFilter(QObject *obj, QEvent *event) {
        switch (event->type()) {
            case QEvent::ActionAdded: {
                auto e = static_cast<QActionEvent *>(event);
                auto w = static_cast<QWidget *>(obj);
                auto a = e->action();
                widgets[w].actions.append(a, 0);

                auto &data = actions[a];
                data.widgets.insert(w);
                data.keys = a->shortcuts();

                postFixTask();
                break;
            }

            case QEvent::ActionRemoved: {
                auto e = static_cast<QActionEvent *>(event);
                auto w = static_cast<QWidget *>(obj);
                auto a = e->action();
                widgets[w].actions.remove(a);

                auto it = actions.find(a);
                auto &widgets = it->widgets;
                widgets.remove(w);
                if (widgets.isEmpty()) {
                    fixing = true;
                    a->setShortcuts(it->keys); // Restore desired keys;
                    fixing = false;

                    actions.erase(it);
                }

                postFixTask();
                break;
            }

            case QEvent::ActionChanged: {
                if (fixing)
                    break;
                auto e = static_cast<QActionEvent *>(event);
                auto a = e->action();

                auto &data = actions[a];
                data.keys = a->shortcuts(); // Update desired keys

                postFixTask();
                break;
            }

            default:
                break;
        }
        return QObject::eventFilter(obj, event);
    }

    void QMShortcutContextPrivate::_q_widgetDestroyed(QObject *obj) {
        removeWidget(static_cast<QWidget *>(obj));
    }

    QMShortcutContext::QMShortcutContext(QObject *parent)
        : QObject(parent), d(new QMShortcutContextPrivate(this)) {
    }

    QMShortcutContext::~QMShortcutContext() {
        delete d;
    }

    void QMShortcutContext::addWidget(QWidget *w, int priority) {
        d->addWidget(w, priority);
    }

    void QMShortcutContext::removeWidget(QWidget *w) {
        d->removeWidget(w);
    }

    QList<QWidget *> QMShortcutContext::widgets() const {
        return d->widgets.keys();
    }

    bool QMShortcutContext::containsWidget(QWidget *w) const {
        return d->widgets.contains(w);
    }

}