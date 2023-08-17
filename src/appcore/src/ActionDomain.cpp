#include "ActionDomain.h"
#include "ActionDomain_p.h"

namespace Core {

#define myWarning(func) (qWarning().nospace() << "Core::ActionDomain::" << (func) << "():").space()

    ActionDomainItemPrivate::ActionDomainItemPrivate(int type) : type(type) {
        domain = nullptr;
    }

    ActionDomainItem::ActionDomainItem(const QString &id, int type) : ActionDomainItem(id, {}, {}, type) {
    }

    ActionDomainItem::ActionDomainItem(const QString &id, const QString &title, const QList<ActionInsertRuleV2> &rules,
                                       int type)
        : d(new ActionDomainItemPrivate(type)) {
        d->id = id;
        d->title = title;
        d->rules = rules;
    }

    ActionDomainItem::~ActionDomainItem() {
    }

    QString ActionDomainItem::id() const {
        return d->id;
    }

    ActionDomain *ActionDomainItem::domain() const {
        return d->domain;
    }

    int ActionDomainItem::type() const {
        return d->type;
    }

    QString ActionDomainItem::title() const {
        return d->title;
    }

    void ActionDomainItem::setTitle(const QMDisplayString &title) {
        d->title = title;
    }

    QList<ActionInsertRuleV2> ActionDomainItem::rules() const {
        return d->rules;
    }

    void ActionDomainItem::setRules(const QList<ActionInsertRuleV2> &rules) {
        d->rules = rules;
    }

    ActionDomainPrivate::ActionDomainPrivate() {
        configurable = true;
        stateDirty = true;
        cachedStateDirty = true;
    }

    ActionDomainPrivate::~ActionDomainPrivate() {
        qDeleteAll(topItems);
        qDeleteAll(actions);
    }

    void ActionDomainPrivate::init() {
    }

    void ActionDomainPrivate::setStateDirty() {
        Q_Q(ActionDomain);

        stateDirty = true;
        cachedStateDirty = true;

        emit q->stateChanged();
    }

    void ActionDomainPrivate::setCachedStateDirty() {
        Q_Q(ActionDomain);

        cachedStateDirty = true;

        emit q->stateChanged();
    }

    ActionDomain::ActionDomain(const QString &id, QObject *parent)
        : ActionDomain(*new ActionDomainPrivate(), id, parent) {
    }

    ActionDomain::ActionDomain(const QString &id, const QMDisplayString &title, QObject *parent)
        : ActionDomain(id, parent) {
        Q_D(ActionDomain);
        d->title = title;
    }

    ActionDomain::~ActionDomain() {
    }

    QString ActionDomain::id() const {
        Q_D(const ActionDomain);
        return d->id;
    }

    QString ActionDomain::title() const {
        Q_D(const ActionDomain);
        return d->title;
    }

    void ActionDomain::setTitle(const QMDisplayString &title) {
        Q_D(ActionDomain);
        d->title = title;
    }

    bool ActionDomain::configurable() const {
        Q_D(const ActionDomain);
        return d->configurable;
    }

    void ActionDomain::setConfigurable(bool configurable) {
        Q_D(ActionDomain);
        d->configurable = configurable;
    }

    bool ActionDomain::addTopLevelItem(ActionDomainItem *item) {
        Q_D(ActionDomain);

        if (!item) {
            myWarning(__func__) << "trying to add null item";
            return false;
        }


        auto &domain = item->d->domain;
        if (domain) {
            if (domain == this)
                myWarning(__func__) << "trying to add duplicated item:" << item->id();
            else
                myWarning(__func__) << "item" << item->id() << "has been added to domain:" << domain;
            return false;
        }

        domain = this;
        d->topItems.append(item->id(), item);
        d->setStateDirty();

        return true;
    }

    bool ActionDomain::removeTopLevelItem(const QString &id) {
        Q_D(ActionDomain);

        auto it = d->topItems.find(id);
        if (it == d->topItems.end()) {
            myWarning(__func__) << "item does not exist:" << id;
            return false;
        }

        auto action = it.value();
        action->d->domain = nullptr;
        d->topItems.erase(it);
        d->setStateDirty();

        return true;
    }

    bool ActionDomain::removeTopLevelItem(ActionDomainItem *item) {
        if (!item) {
            myWarning(__func__) << "trying to remove null item";
            return false;
        }
        return removeTopLevelItem(item->id());
    }

    QList<ActionDomainItem *> ActionDomain::topLevelItems() const {
        Q_D(const ActionDomain);
        return d->topItems.values();
    }

    bool ActionDomain::addAction(ActionDomainItem *item) {
        Q_D(ActionDomain);

        if (!item) {
            myWarning(__func__) << "trying to add null item";
            return false;
        }

        auto &domain = item->d->domain;
        if (domain) {
            if (domain == this)
                myWarning(__func__) << "trying to add duplicated item:" << item->id();
            else
                myWarning(__func__) << "item" << item->id() << "has been added to domain:" << domain;
            return false;
        }

        domain = this;
        d->actions.append(item->id(), item);
        d->setStateDirty();

        return true;
    }

    bool ActionDomain::removeAction(const QString &id) {
        Q_D(ActionDomain);

        auto it = d->actions.find(id);
        if (it == d->actions.end()) {
            myWarning(__func__) << "item does not exist:" << id;
            return false;
        }

        auto action = it.value();
        action->d->domain = nullptr;
        d->actions.erase(it);
        d->setStateDirty();

        return true;
    }

    bool ActionDomain::removeAction(ActionDomainItem *item) {
        Q_D(ActionDomain);

        if (!item) {
            myWarning(__func__) << "trying to add null item";
            return false;
        }

        return removeAction(item->id());
    }

    QList<ActionDomainItem *> ActionDomain::actions() const {
        Q_D(const ActionDomain);
        return d->actions.values();
    }

    ActionDomainState ActionDomain::state() const {
        Q_D(const ActionDomain);
        if (d->stateDirty) {
            // TODO:

            d->stateDirty = false;
        }
        return d->state;
    }

    ActionDomainState ActionDomain::cachedState() const {
        Q_D(const ActionDomain);
        if (d->cachedStateDirty) {
            // TODO:

            auto state = this->state();
            d->cachedStateDirty = false;
        }
        return d->cachedState;
    }

    void ActionDomain::buildDomain(const QMap<QString, QWidget *> &topLevelMenus,
                                   const QList<Core::ActionItem *> &items) const {
    }

    ActionDomain::ActionDomain(ActionDomainPrivate &d, const QString &id, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.id = id;

        d.init();
    }
}
