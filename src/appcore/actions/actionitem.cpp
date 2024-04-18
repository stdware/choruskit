#include "actionitem.h"

#include <QDebug>
#include <utility>

namespace Core {

#define myWarning(func) qWarning() << "Core::ActionItem(): "

    class ActionItemPrivate : public QObject {
        Q_DECLARE_PUBLIC(ActionItem)
    public:
        ActionItemPrivate();
        virtual ~ActionItemPrivate();

        void init();

        ActionItem *q_ptr;

        QString id;
        ActionItem::Type type;

        QAction *action;

        QWidgetAction *widgetAction;

        ActionItem::MenuFactory menuFactory;
        QSet<QMenu *> createdMenus;

        QWidget *topLevelWidget;

    private:
        void _q_menuDestroyed(QObject *obj) {
            // Q_Q(ActionItem);
            auto menu = static_cast<QMenu *>(obj);
            // Q_EMIT q->menuDestroyed(menu);
            createdMenus.remove(menu);
        }
    };

    ActionItemPrivate::ActionItemPrivate() : q_ptr(nullptr) {
        type = ActionItem::Action;
        action = nullptr;
        widgetAction = nullptr;
        topLevelWidget = nullptr;
    }

    ActionItemPrivate::~ActionItemPrivate() {
        switch (type) {
            case ActionItem::Widget:
                delete widgetAction;
                break;
            case ActionItem::Menu: {
                for (const auto &menu : std::as_const(createdMenus)) {
                    connect(menu, &QObject::destroyed, this, &ActionItemPrivate::_q_menuDestroyed);
                }
                auto menusToDelete = createdMenus;
                createdMenus.clear();
                qDeleteAll(menusToDelete);
                break;
            }
            default:
                break;
        }
    }

    void ActionItemPrivate::init() {
    }

    ActionItem::ActionItem(const QString &id, QAction *action, QObject *parent)
        : ActionItem(*new ActionItemPrivate(), id, parent) {
        Q_D(ActionItem);
        d->type = Action;
        d->action = action;
        // action->setProperty("action-id", id);
    }

    namespace {

        class WidgetAction : public QWidgetAction {
        public:
            explicit WidgetAction(ActionItem::WidgetFactory factory, const QString &id,
                                  QObject *parent = nullptr)
                : QWidgetAction(parent), fac(std::move(factory)) {
            }

        protected:
            QWidget *createWidget(QWidget *parent) override {
                auto w = fac(parent);
                // w->setProperty("action-id", id);
                return w;
            }

            ActionItem::WidgetFactory fac;
            QString id;

            friend class Core::ActionItem;
        };

    }

    ActionItem::ActionItem(const QString &id, const WidgetFactory &fac, QObject *parent) {
        Q_D(ActionItem);
        d->type = Widget;
        d->widgetAction = new WidgetAction(fac, id, this);
    }

    ActionItem::ActionItem(const QString &id, const MenuFactory &fac, QObject *parent)
        : ActionItem(*new ActionItemPrivate(), id, parent) {
        Q_D(ActionItem);
        d->type = Menu;
        d->menuFactory = fac;
    }

    ActionItem::ActionItem(const QString &id, QWidget *topLevelWidget, QObject *parent) {
        Q_D(ActionItem);
        d->type = TopLevel;
        d->topLevelWidget = topLevelWidget;
    }

    ActionItem::~ActionItem() = default;

    QString ActionItem::id() const {
        Q_D(const ActionItem);
        return d->id;
    }

    ActionItem::Type ActionItem::type() const {
        Q_D(const ActionItem);
        return d->type;
    }

    QAction *ActionItem::action() const {
        Q_D(const ActionItem);
        return d->action;
    }

    QWidgetAction *ActionItem::widgetAction() const {
        Q_D(const ActionItem);
        return d->widgetAction;
    }

    QWidget *ActionItem::topLevel() const {
        Q_D(const ActionItem);
        return d->topLevelWidget;
    }

    QList<QWidget *> ActionItem::createdWidgets() const {
        Q_D(const ActionItem);
        return d->widgetAction ? static_cast<WidgetAction *>(d->widgetAction)->createdWidgets()
                               : QList<QWidget *>();
    }

    QList<QMenu *> ActionItem::createdMenus() const {
        Q_D(const ActionItem);
        return d->createdMenus.values();
    }

    QMenu *ActionItem::requestMenu(QWidget *parent) {
        Q_D(ActionItem);
        
        auto menu = d->menuFactory(parent);
        // Q_EMIT menuCreated(menu);
        connect(menu, &QObject::destroyed, d, &ActionItemPrivate::_q_menuDestroyed);
        d->createdMenus.insert(menu);
        return menu;
    }

    ActionItem::ActionItem(ActionItemPrivate &d, const QString &id, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.id = id;

        d.init();
    }

}