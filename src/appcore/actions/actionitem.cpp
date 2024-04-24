#include "actionitem.h"
#include "actionitem_p.h"

#include <utility>

#include <QDebug>

namespace Core {

#define myWarning(func) qWarning() << "Core::ActionItem(): "

    ActionItemPrivate::ActionItemPrivate() : q_ptr(nullptr) {
        type = ActionItem::Action;
        action = nullptr;
        sharedWidgetAction = nullptr;
        topLevelWidget = nullptr;
    }

    ActionItemPrivate::~ActionItemPrivate() {
        Q_Q(ActionItem);
        switch (type) {
            case ActionItem::Action:
                delete action;
                break;
            case ActionItem::Widget:
                delete sharedWidgetAction;
                break;
            case ActionItem::Menu: {
                deleteAllMenus();
                break;
            }
            default:
                break;
        }
    }

    void ActionItemPrivate::init() {
    }

    void ActionItemPrivate::deleteAllMenus() {
        if (createdMenus.isEmpty())
            return;
        for (const auto &menu : std::as_const(createdMenus)) {
            disconnect(menu.data(), &QObject::destroyed, this,
                       &ActionItemPrivate::_q_menuDestroyed);
        }
        auto menusToDelete = createdMenus;
        createdMenus.clear();
        qDeleteAll(menusToDelete);
    }

    void ActionItemPrivate::_q_menuDestroyed(QObject *obj) {
        // Q_Q(ActionItem);
        auto menu = static_cast<QMenu *>(obj);
        // Q_EMIT q->menuDestroyed(menu);
        createdMenus.removeAll(menu);
    }

    ActionItem::ActionItem(const QString &id, QAction *action, QObject *parent)
        : ActionItem(*new ActionItemPrivate(), id, parent) {
        Q_D(ActionItem);
        d->type = Action;
        d->action = action;
        // action->setProperty("action-id", id);
    }

    class ActionItemWidgetAction : public QWidgetAction {
    public:
        explicit ActionItemWidgetAction(ActionItem::WidgetFactory factory, const QString &id,
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

        friend class ActionItem;
    };


    ActionItem::ActionItem(const QString &id, const WidgetFactory &fac, QObject *parent)
        : ActionItem(*new ActionItemPrivate(), id, parent) {
        Q_D(ActionItem);
        d->type = Widget;
        d->sharedWidgetAction = new ActionItemWidgetAction(fac, id, this);
    }

    ActionItem::ActionItem(const QString &id, const MenuFactory &fac, QObject *parent)
        : ActionItem(*new ActionItemPrivate(), id, parent) {
        Q_D(ActionItem);
        d->type = Menu;
        d->menuFactory = fac;
    }

    ActionItem::ActionItem(const QString &id, QWidget *topLevelWidget, QObject *parent)
        : ActionItem(*new ActionItemPrivate(), id, parent) {
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
        return d->sharedWidgetAction;
    }

    QWidget *ActionItem::topLevel() const {
        Q_D(const ActionItem);
        return d->topLevelWidget;
    }

    QList<QWidget *> ActionItem::createdWidgets() const {
        Q_D(const ActionItem);
        return d->sharedWidgetAction
                   ? static_cast<ActionItemWidgetAction *>(d->sharedWidgetAction)->createdWidgets()
                   : QList<QWidget *>();
    }

    QList<QMenu *> ActionItem::createdMenus() const {
        Q_D(const ActionItem);
        QList<QMenu *> menus;
        menus.reserve(d->createdMenus.size());
        for (const auto &item : d->createdMenus) {
            menus.append(item);
        }
        return menus;
    }

    QMenu *ActionItem::requestMenu(QWidget *parent) {
        Q_D(ActionItem);
        if (!d->menuFactory)
            return nullptr;
        auto menu = d->menuFactory(parent);
        addMenuAsRequested(menu);
        return menu;
    }

    void ActionItem::addMenuAsRequested(QMenu *menu) {
        Q_D(ActionItem);

        // Q_EMIT menuCreated(menu);
        connect(menu, &QObject::destroyed, d, &ActionItemPrivate::_q_menuDestroyed);
        d->createdMenus.append(menu);
    }

    ActionItem::ActionItem(ActionItemPrivate &d, const QString &id, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.id = id;

        d.init();
    }

}
