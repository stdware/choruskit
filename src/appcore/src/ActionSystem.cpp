#include "ActionSystem.h"
#include "ActionSystem_p.h"

#include "ActionDomain_p.h"

#include "QMChronSet.h"
#include "QMXmlAdaptor.h"

#include "QMBatch.h"

#include "ILoader.h"

#include <QDebug>
#include <QFile>

namespace Core {

#define myWarning(func) (qWarning().nospace() << "Core::ActionSystem::" << (func) << "():").space()

    ActionSystemPrivate::ActionSystemPrivate() : q_ptr(nullptr) {
    }

    ActionSystemPrivate::~ActionSystemPrivate() {
    }

    void ActionSystemPrivate::init() {
        readSettings();
        configVars.addHash(QMSimpleVarExp::SystemValues());
    }

    static ActionSystem *m_instance = nullptr;

    ActionSystem::ActionSystem(QObject *parent) : ActionSystem(*new ActionSystemPrivate(), parent) {
    }

    ActionSystem::~ActionSystem() {
        Q_D(ActionSystem);
        d->saveSettings();

        m_instance = nullptr;
    }

    bool ActionSystem::addAction(ActionSpec *action) {
        Q_D(ActionSystem);
        if (!action) {
            myWarning(__func__) << "trying to add null action";
            return false;
        }
        if (d->actions.contains(action->id())) {
            myWarning(__func__) << "trying to add duplicated action:" << action->id();
            return false;
        }
        action->setParent(this);
        d->actions.append(action->id(), action);

        return true;
    }

    bool ActionSystem::removeAction(Core::ActionSpec *action) {
        if (action == nullptr) {
            myWarning(__func__) << "trying to remove null action";
            return false;
        }
        return removeAction(action->id());
    }

    bool ActionSystem::removeAction(const QString &id) {
        Q_D(ActionSystem);
        auto it = d->actions.find(id);
        if (it == d->actions.end()) {
            myWarning(__func__) << "action does not exist:" << id;
            return false;
        }

        auto action = it.value();
        action->setParent(nullptr);
        d->actions.erase(it);

        return true;
    }

    ActionSpec *ActionSystem::action(const QString &id) const {
        Q_D(const ActionSystem);
        return d->actions.value(id, nullptr);
    }

    QList<ActionSpec *> ActionSystem::actions() const {
        Q_D(const ActionSystem);
        return d->actions.values();
    }

    QStringList ActionSystem::actionIds() const {
        Q_D(const ActionSystem);
        return d->actions.keys();
    }

    bool ActionSystem::addDomain(ActionDomain *domain) {
        Q_D(ActionSystem);
        if (!domain) {
            myWarning(__func__) << "trying to add null domain";
            return false;
        }
        if (d->domains.contains(domain->id())) {
            myWarning(__func__) << "trying to add duplicated domain:" << domain->id();
            return false;
        }
        domain->setParent(this);
        d->domains.append(domain->id(), domain);

        return true;
    }

    bool ActionSystem::removeDomain(ActionDomain *domain) {
        if (domain == nullptr) {
            myWarning(__func__) << "trying to remove null domain";
            return false;
        }
        return removeDomain(domain->id());
    }

    bool ActionSystem::removeDomain(const QString &id) {
        Q_D(ActionSystem);
        auto it = d->domains.find(id);
        if (it == d->domains.end()) {
            myWarning(__func__) << "context does not exist:" << id;
            return false;
        }

        auto context = it.value();
        context->setParent(nullptr);
        d->domains.erase(it);

        return true;
    }

    ActionDomain *ActionSystem::domain(const QString &id) const {
        Q_D(const ActionSystem);
        return d->domains.value(id, nullptr);
    }

    QList<ActionDomain *> ActionSystem::domains() const {
        Q_D(const ActionSystem);
        return d->domains.values();
    }

    QStringList ActionSystem::domainIds() const {
        Q_D(const ActionSystem);
        return d->domains.keys();
    }

    static void getIcon(QIcon &icon, const QString &iconArg) {
        if (iconArg.isNull()) {
            return;
        }

        // Extract icon
        if (iconArg.startsWith("svg(") && iconArg.endsWith(')')) {
            icon = QIcon(iconArg.mid(4, iconArg.size() - 5) + ", .svgx");
        } else {
            icon = QIcon(iconArg);
        }
    }

    // void ActionSystemPrivate::loadContexts_dfs(const QString &prefix, const QString &parentId,
    //                                            const QMXmlAdaptorElement *ele, ActionContext *context) {
    //     Q_Q(ActionSystem);

    //     const auto &ctx2 = *ele;

    //     bool isMenu = ctx2.name == "menu";
    //     bool isAction = ctx2.name == "action";
    //     bool isSeparator = ctx2.name == "separator";
    //     bool isGroup = isMenu || ctx2.name == "group";

    //     QString id = ctx2.properties.value("id");
    //     if (!isSeparator) {
    //         if (id.isEmpty())
    //             return;

    //         if (id.startsWith('^')) {
    //             id.remove(0, 1);
    //         } else {
    //             id.prepend(prefix);
    //         }
    //     }

    //     QList<ActionInsertRule> rules;
    //     QList<QKeySequence> shortcuts;
    //     QString commandName;
    //     QIcon icon;
    //     if (!isGroup && !isSeparator) {
    //         if (!isAction) {
    //             return;
    //         }

    //         QString key = ctx2.properties.value("shortcut");
    //         if (!key.isEmpty()) {
    //             shortcuts.append(QKeySequence(key));
    //         }

    //         key = ctx2.properties.value("command");
    //         if (!key.isEmpty() && commandName.isEmpty()) {
    //             commandName = configVars.parse(key);
    //         }
    //     }

    //     if (!isGroup || isMenu) {
    //         getIcon(icon, ctx2.properties.value("icon"));
    //     }

    //     if (!parentId.isEmpty()) {
    //         rules.append({parentId, ActionInsertRule::Append});
    //     }

    //     QList<QMXmlAdaptorElement *> children;
    //     for (const auto &ctx_ref3 : qAsConst(ctx2.children)) {
    //         const auto &ctx3 = *ctx_ref3;
    //         if (ctx3.name == "rules") {
    //             if (rules.isEmpty()) {
    //                 for (const auto &ctx_ref4 : qAsConst(ctx3.children)) {
    //                     const auto &ctx4 = *ctx_ref4;
    //                     if (ctx4.name != "rule") {
    //                         continue;
    //                     }

    //                     QString targetId = ctx4.properties.value("id");
    //                     if (targetId.isEmpty()) {
    //                         continue;
    //                     }

    //                     auto mode = ctx4.properties.value("mode");
    //                     ActionInsertRule::InsertMode rule = ActionInsertRule::Append;
    //                     if (!mode.isEmpty()) {
    //                         if (mode == "prepend" || mode == "unshift") {
    //                             rule = ActionInsertRule::Unshift;
    //                         } else if (mode == "insertBehind") {
    //                             rule = ActionInsertRule::InsertBehind;
    //                         } else if (mode == "insertFront") {
    //                             rule = ActionInsertRule::InsertFront;
    //                         }
    //                     }

    //                     rules.append({targetId, rule});
    //                 }
    //             }
    //         } else if (ctx3.name == "shortcuts") {
    //             if (isAction) {
    //                 for (const auto &ctx_ref4 : qAsConst(ctx3.children)) {
    //                     const auto &ctx4 = *ctx_ref4;
    //                     if (ctx4.name != "shortcut" || ctx4.value.isEmpty()) {
    //                         continue;
    //                     }
    //                     shortcuts.append(ctx4.value);
    //                 }
    //             }
    //         } else if (ctx3.name == "icon") {
    //             if (isAction || isMenu) {
    //                 getIcon(icon, ctx3.value);
    //             }
    //         } else if (ctx3.name == "content") {
    //             if (isGroup) {
    //                 for (const auto &ctx_ref4 : qAsConst(ctx3.children)) {
    //                     children.append(ctx_ref4.data());
    //                 }
    //             }
    //         } else if (isGroup) {
    //             children.append(ctx_ref3.data());
    //         }
    //     }

    //     if (!isSeparator) {
    //         auto action = context->action(id);
    //         if (!action) {
    //             action = context->addAction(id, isGroup);
    //         }
    //         action.setRules(action.rules() + rules);

    //         auto it = actions.find(id);
    //         if (it == actions.end()) {
    //             auto spec = new ActionSpec(id);
    //             spec->setShortcuts(shortcuts);
    //             spec->setCommandName(commandName);
    //             if (!icon.isNull()) {
    //                 spec->setIcon(icon);
    //             }
    //             q->addAction(spec);
    //         }
    //     } else {
    //         auto action = context->addSeparator();
    //         action.setRules(action.rules() + rules);
    //     }

    //     for (const auto &child : qAsConst(children)) {
    //         loadContexts_dfs(prefix, id, child, context);
    //     }
    // }

    static const char settingCatalogC[] = "ActionSystem";

    static const char stateGroupC[] = "State";

    QMap<QString, QStringList> _obj_to_state(const QJsonObject &obj) {
        QMap<QString, QStringList> res;
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            if (!it->isArray()) {
                continue;
            }
            res.insert(it.key(), QMBatch::arrayToStringList(it->toArray()));
        }
        return res;
    }

    QJsonObject _state_to_obj(const QMap<QString, QStringList> &state) {
        QJsonObject obj;
        for (auto it = state.begin(); it != state.end(); ++it) {
            obj.insert(it.key(), QJsonArray::fromStringList(it.value()));
        }
        return obj;
    }

    void ActionSystemPrivate::readSettings() {
        stateCaches.clear();

        auto settings = ILoader::instance()->settings();

        auto obj = settings->value(settingCatalogC).toObject();
        auto stateObj = obj.value(stateGroupC).toObject();
        for (auto it = stateObj.begin(); it != stateObj.end(); ++it) {
            if (!it->isObject()) {
                continue;
            }
            auto state = _obj_to_state(it->toObject());
            if (state.isEmpty()) {
                continue;
            }
            stateCaches.insert(it.key(), state);
        }
    }

    void ActionSystemPrivate::saveSettings() const {
        auto settings = ILoader::instance()->settings();

        QJsonObject stateObj;
        for (auto it = stateCaches.begin(); it != stateCaches.end(); ++it) {
            stateObj.insert(it.key(), _state_to_obj(it.value()));
        }

        QJsonObject obj;
        obj.insert(stateGroupC, stateObj);

        settings->insert(settingCatalogC, obj);
    }

    QStringList ActionSystem::loadContexts(const QString &filename) {
        Q_D(ActionSystem);

        // QMXmlAdaptor file;
        // if (!file.load(filename)) {
        //     myWarning(__func__) << "load file failed";
        //     return {};
        // }

        // const auto &root = file.root;
        // if (root.name != "actionSystem") {
        //     return {};
        // }

        // QMChronSet<QString> res;
        // for (const auto &ctx_ref : qAsConst(root.children)) {
        //     const auto &ctx = *ctx_ref;
        //     if (ctx.name != "context") {
        //         continue;
        //     }

        //     QString id = ctx.properties.value("id");
        //     if (id.isEmpty()) {
        //         continue;
        //     }

        //     ActionContext *context = nullptr;
        //     if (ctx.properties.value("mode") == "find") {
        //         context = this->context(id);
        //     } else if (!this->context(id)) {
        //         context = new ActionContext(id);
        //         addContext(context);
        //     }

        //     if (!context) {
        //         continue;
        //     }
        //     res.append(id);

        //     QString prefix = ctx.properties.value("prefix");
        //     for (const auto &ctx_ref2 : qAsConst(ctx.children)) {
        //         d->loadContexts_dfs(prefix, {}, ctx_ref2.data(), context);
        //     }
        // }

        // return res.values();

        return {};
    }

    QStringList ActionSystem::loadDomains(const QString &fileName) {
        return {};
    }

    QMap<QString, QStringList> ActionSystem::stateCache(const QString &domainId) {
        Q_D(const ActionSystem);
        return d->stateCaches.value(domainId, {});
    }

    void ActionSystem::setStateCache(const QString &domainId, const ActionDomainState &state) {
        Q_D(ActionSystem);
        d->stateCaches.insert(domainId, state);

        auto dom = domain(domainId);
        if (dom) {
            dom->d_func()->setCachedStateDirty();
        }
    }

    QList<QKeySequence> ActionSystem::shortcutsCache(const QString &actionId) {
        Q_D(const ActionSystem);
        return d->shortcutsCaches.value(actionId, {});
    }

    void ActionSystem::setShortcutsCache(const QString &actionId, const QList<QKeySequence> &shortcutsCache) {
        Q_D(ActionSystem);
        d->shortcutsCaches.insert(actionId, shortcutsCache);

        auto action = this->action(actionId);
        if (action) {
            emit action->shortcutsChanged();
        }
    }

    ActionIconSpec ActionSystem::iconCache(const QString &actionId) {
        Q_D(const ActionSystem);
        return d->iconCaches.value(actionId, {});
    }

    void ActionSystem::setIconCache(const QString &actionId, const ActionIconSpec &iconCache) {
        Q_D(ActionSystem);
        d->iconCaches.insert(actionId, iconCache);

        auto action = this->action(actionId);
        if (action) {
            emit action->iconChanged();
        }
    }

    ActionSystem::ActionSystem(ActionSystemPrivate &d, QObject *parent) : QObject(parent), d_ptr(&d) {
        m_instance = this;

        d.q_ptr = this;
        d.init();
    }

}