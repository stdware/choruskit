#include "ActionDomain.h"
#include "ActionDomain_p.h"

#include "ICoreBase.h"

namespace Core {

#define myWarning(func) (qWarning().nospace() << "Core::ActionDomain::" << (func) << "():").space()

    class StretchWidgetAction : public QWidgetAction {
    public:
        explicit StretchWidgetAction(QObject *parent = nullptr) : QWidgetAction(parent) {
        }

    protected:
        QWidget *createWidget(QWidget *parent) override {
            auto w = new QWidget(parent);
            w->setDisabled(true);
            w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            return w;
        }
    };


    ActionDomainPrivate::ActionDomainPrivate() {
        configurable = true;
        stateDirty = true;
        cachedStateDirty = true;
        sharedWidgetAction = nullptr;
    }

    ActionDomainPrivate::~ActionDomainPrivate() {
    }

    void ActionDomainPrivate::init() {
        Q_Q(ActionDomain);
        sharedWidgetAction = new StretchWidgetAction(q);
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

    struct RuleData {
        ActionDomain::Type type;
        bool expandMenu;
    };

    static QHash<QString, QMChronMap<QString, RuleData>> parseStateToMap(const ActionDomainState &state) {
        QHash<QString, QMChronMap<QString, RuleData>> map;
        int randomIdCount = 0;
        for (auto it = state.begin(); it != state.end(); ++it) {
            const auto &targetId = it.key();
            const auto &stateList = it.value();

            auto &stateMap = map[targetId];
            for (const auto &s : stateList) {
                int index = s.indexOf(':');
                if (index < 0) {
                    continue;
                }

                RuleData rd{ActionDomain::Action, false};
                QString id = s.mid(0, index);
                const auto &type = s.midRef(index + 1);
                if (type.startsWith("group")) {
                    rd.type = ActionDomain::Group;
                } else if (type.startsWith("menu")) {
                    rd.type = ActionDomain::Menu;

                    int index2 = s.indexOf(':', index + 1);
                    if (index2 >= 0 && s.midRef(index2 + 1) == "expand") {
                        rd.expandMenu = true;
                    }
                } else if (type.startsWith("separator")) {
                    rd.type = ActionDomain::Separator;
                    id = "_tmp_" + QString::number(++randomIdCount);
                } else if (type.startsWith("stretch")) {
                    rd.type = ActionDomain::Stretch;
                    id = "_tmp_" + QString::number(++randomIdCount);
                }
                stateMap.append(id, rd);
            }
        }
        return map;
    }

    ActionDomainState ActionDomainPrivate::calcState(const ActionDomainState &existingState,
                                                     const decltype(rules) &rules) const {
        // Build temporary state
        QHash<QString, QMChronMap<QString, RuleData>> myState = parseStateToMap(existingState);

        for (auto rule_iterator = rules.begin(); rule_iterator != rules.end(); ++rule_iterator) {
            const auto &targetId = rule_iterator.key();

            // Check target existence
            if (targetId.startsWith('%')) {
                if (!topLevelMenus.contains(targetId.mid(1))) {
                    continue;
                }
            } else {
                auto it = actions.find(targetId);
                if (it == actions.end())
                    continue;
                if (it.value() != ActionDomain::Group && it.value() != ActionDomain::Menu)
                    continue;
            }

            auto &stateMap = myState[targetId];
            const auto &ruleMap = rule_iterator.value();

            for (auto ruleMap_iterator = ruleMap.begin(); ruleMap_iterator != ruleMap.end(); ++ruleMap_iterator) {
                const auto &id = ruleMap_iterator.key();
                if (stateMap.contains(id)) {
                    // This action has been inserted in the existing state
                    continue;
                }

                if (!actions.contains(id)) {
                    // Invalid action
                    continue;
                }

                const auto &rule = ruleMap_iterator.value();

                const auto &refItem = rule.refItem;
                decltype(myState)::mapped_type ::const_iterator target_iterator;

                // Find insert position
                if (!refItem.isEmpty()) {
                    auto it1 = stateMap.find(refItem);
                    if (it1 == stateMap.end()) {
                        continue;
                    }

                    if (rule.mode == ActionInsertRule::Append) {
                        target_iterator = it1;
                    } else {
                        target_iterator = (it1 != stateMap.begin()) ? std::prev(it1) : it1;
                    }
                } else {
                    if (rule.mode == ActionInsertRule::Append) {
                        target_iterator = stateMap.end();
                    } else {
                        target_iterator = stateMap.begin();
                    }
                }

                stateMap.insert(target_iterator, id,
                                RuleData{
                                    actions.value(id),
                                    rule.expandMenu,
                                });
            }
        }

        // Convert to state map
        ActionDomainState state;
        for (auto it = myState.begin(); it != myState.end(); ++it) {
            QStringList list;
            for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
                switch (it2->type) {
                    case ActionDomain::Action:
                        list.append(it2.key() + ":action");
                        break;
                    case ActionDomain::Group:
                        list.append(it2.key() + ":group");
                        break;
                    case ActionDomain::Menu:
                        list.append(it2.key() + ":menu" + QString(it2->expandMenu ? ":expand" : ""));
                        break;
                    case ActionDomain::Separator:
                        list.append(":separator");
                        break;
                    case ActionDomain::Stretch:
                        list.append(":stretch");
                        break;
                }
            }
            state.insert(it.key(), list);
        }

        return state;
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

    bool ActionDomain::addTopLevelMenu(const QString &id) {
        Q_D(ActionDomain);
        if (d->topLevelMenus.contains(id)) {
            myWarning(__func__) << "trying to add duplicated top level menu" << id;
            return false;
        }
        d->topLevelMenus.insert(id);
        return true;
    }

    bool ActionDomain::removeTopLevelMenu(const QString &id) {
        Q_D(ActionDomain);
        if (!d->topLevelMenus.remove(id)) {
            myWarning(__func__) << "top level menu doesn't exist" << id;
            return false;
        }
        return true;
    }

    bool ActionDomain::containsTopLevelMenu(const QString &id) const {
        Q_D(const ActionDomain);
        return d->topLevelMenus.contains(id);
    }

    QStringList ActionDomain::topLevelMenus() const {
        Q_D(const ActionDomain);
        return d->topLevelMenus.values();
    }

    bool ActionDomain::addAction(const QString &id, ActionDomain::Type type) {
        Q_D(ActionDomain);
        if (d->actions.contains(id)) {
            myWarning(__func__) << "trying to add duplicated top action" << id;
            return false;
        }
        d->actions.insert(id, type);
        return true;
    }

    bool ActionDomain::removeAction(const QString &id) {
        Q_D(ActionDomain);
        if (!d->actions.remove(id)) {
            myWarning(__func__) << "top level menu doesn't exist" << id;
            return false;
        }
        return true;
    }

    bool ActionDomain::containsAction(const QString &id) const {
        Q_D(const ActionDomain);
        return d->actions.contains(id);
    }

    std::optional<ActionDomain::Type> ActionDomain::actionType(const QString &id) const {
        Q_D(const ActionDomain);
        auto it = d->actions.find(id);
        if (it == d->actions.end())
            return {};
        return *it;
    }

    QStringList ActionDomain::actions() const {
        Q_D(const ActionDomain);
        return d->actions.keys();
    }

    void ActionDomain::addInsertRule(const QString &targetId, const QString &id, const ActionInsertRule &rule) {
        Q_D(ActionDomain);
        auto it = d->rules.find(targetId);
        if (it != d->rules.end()) {
            auto &idIndexes = it.value();
            auto it2 = idIndexes.find(id);
            if (it2 != idIndexes.end()) {
                // Replace
                it2.value() = rule;
                goto out;
            }

            // Insert
            idIndexes.append(id, rule);
            goto out;
        }

        // Add index
        d->rules.insert(targetId, {
                                      {id, rule}
        });

    out:
        d->setStateDirty();
    }

    void ActionDomain::removeInsertRule(const QString &targetId, const QString &id) {
        Q_D(ActionDomain);
        auto it = d->rules.find(targetId);
        if (it == d->rules.end()) {
            return;
        }

        auto &idIndexes = it.value();
        if (!idIndexes.remove(id)) {
            return;
        }

        if (idIndexes.isEmpty()) {
            d->rules.erase(it);
        }
        d->setStateDirty();
    }

    bool ActionDomain::hasActionInsertRule(const QString &targetId, const QString &id) const {
        Q_D(const ActionDomain);
        return d->rules.value(targetId).contains(id);
    }

    ActionDomainState ActionDomain::state() const {
        Q_D(const ActionDomain);
        if (d->stateDirty) {
            d->state = d->calcState({}, d->rules);
            d->stateDirty = false;
        }
        return d->state;
    }

    ActionDomainState ActionDomain::cachedState() const {
        Q_D(const ActionDomain);
        if (d->cachedStateDirty) {
            auto system = qobject_cast<ActionSystem *>(parent());
            d->cachedState = d->calcState(system ? system->stateCache(d->id) : ActionDomainState{}, d->rules);
            d->cachedStateDirty = false;
        }
        return d->cachedState;
    }

    template <class MenuClass>
    static void recursiveFillActions(const ActionDomainPrivate *d, MenuClass *menu, const QString &id,
                                     const RuleData &rd, QSet<QString> &idSet,
                                     const QHash<QString, QMChronMap<QString, RuleData>> &myState,
                                     const QHash<QString, ActionItem *> &itemMap) {
        if (idSet.contains(id)) // Loop is prohibited
            return;
        idSet.insert(id);

        if (rd.type != ActionDomain::Separator && rd.type != ActionDomain::Stretch) {
            auto it2 = d->actions.find(id);
            if (it2 == d->actions.end() || rd.type != it2.value())
                return;
        }

        auto fillGroup = [&]() {
            if (!menu->actions().back()->isSeparator()) {
                menu->addSeparator();
            }

            auto children2 = myState.value(id);
            for (auto it = children2.begin(); it != children2.end(); ++it) {
                recursiveFillActions(d, menu, it.key(), it.value(), idSet, myState, itemMap);
            }

            if (!menu->actions().back()->isSeparator()) {
                menu->addSeparator();
            }
        };

        switch (rd.type) {
            case ActionDomain::Menu: {
                auto item = itemMap.value(id);
                if (item && item->isMenu() && rd.type == ActionDomain::Menu) {
                    if (rd.expandMenu) {
                        fillGroup();
                    } else {
                        menu->addAction(item->menu()->menuAction());
                    }
                }
                break;
            }
            case ActionDomain::Action: {
                auto item = itemMap.value(id);
                if (item && rd.type == ActionDomain::Action) {
                    if (item->isAction())
                        menu->addAction(item->action());
                    else if (item->isWidget())
                        menu->addAction(item->widgetAction());
                }
                break;
            }
            case ActionDomain::Separator:
                menu->addSeparator();
                break;
            case ActionDomain::Stretch: {
                menu->addAction(d->sharedWidgetAction);
                break;
            }
            case ActionDomain::Group:
                fillGroup();
                break;
        }
    };

    void ActionDomain::buildDomain(const QMap<QString, QWidget *> &topLevelMenus, const QList<ActionItem *> &items,
                                   const ActionDomainState &state) const {
        Q_D(const ActionDomain);

        QHash<QString, ActionItem *> itemMap;
        for (const auto &item : items) {
            itemMap.insert(item->id(), item);
        }
        auto myState = parseStateToMap(state);
        for (auto it = myState.begin(); it != myState.end(); ++it) {
            const auto &targetId = it.key();
            const auto &children = it.value();

            QWidget *w;
            if (targetId.startsWith('%')) {
                if (!d->topLevelMenus.contains(targetId.mid(1)))
                    continue;

                w = topLevelMenus.value(targetId.mid(1), nullptr);
            } else {
                auto it0 = d->actions.find(targetId);
                if (it0 == d->actions.end() || it0.value() != Menu)
                    continue;

                w = itemMap.value(targetId)->menu();
            }

            if (!w)
                continue;

            for (auto it1 = children.begin(); it1 != children.end(); ++it1) {
                const QString &id = it1.key();
                const auto &rd = it1.value();
                QSet<QString> idSet;
                if (w->inherits("QMenu")) {
                    recursiveFillActions(d, static_cast<QMenu *>(w), id, rd, idSet, myState, itemMap);
                } else if (w->inherits("QToolBar")) {
                    recursiveFillActions(d, static_cast<QToolBar *>(w), id, rd, idSet, myState, itemMap);
                } else if (w->inherits("QMenuBar")) {
                    recursiveFillActions(d, static_cast<QMenuBar *>(w), id, rd, idSet, myState, itemMap);
                }
            }
        }
    }

    ActionDomain::ActionDomain(ActionDomainPrivate &d, const QString &id, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.id = id;

        d.init();
    }
}
