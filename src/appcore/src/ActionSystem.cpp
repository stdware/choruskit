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
        configVars.addHash(QMSimpleVarExp::systemValues());
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

    static const char settingCatalogC[] = "ActionSystem";

    static const char stateGroupC[] = "State";

    static const char shortcutsGroupC[] = "Shortcuts";

    static const char iconGroupC[] = "Icon";

    static ActionDomainState __obj_to_state(const QJsonObject &obj) {
        ActionDomainState res;
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            if (!it->isArray()) {
                continue;
            }
            res.insert(it.key(), QMBatch::jsonArrayToStrList(it->toArray()));
        }
        return res;
    }

    static QJsonObject __state_to_obj(const ActionDomainState &state) {
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
            auto state = __obj_to_state(it->toObject());
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
            stateObj.insert(it.key(), __state_to_obj(it.value()));
        }

        QJsonObject obj;
        obj.insert(stateGroupC, stateObj);

        settings->insert(settingCatalogC, obj);
    }

    // Recognize semicolon as the delimiter, you should use two consecutive semicolon to unescape
    static QList<QKeySequence> parseShortcuts(const QString &text) {
        QString curText;
        QList<QKeySequence> res;
        for (auto it = text.begin(); it != text.end(); ++it) {
            const auto &ch = *it;
            if (ch == ';') {
                if (it != text.end() && *(it + 1) == ';') {
                    it++;
                    curText += ";";
                } else {
                    res.append(QKeySequence(curText));
                    curText.clear();
                }
            } else {
                curText += ch;
            }
        }
        if (!curText.isEmpty()) {
            res.append(QKeySequence(curText));
        }
        return res;
    }

    static ActionDomain::Type parseActionDomainType(const QString &name) {
        ActionDomain::Type type = ActionDomain::Action;
        if (name == "group") {
            type = ActionDomain::Group;
        } else if (name == "separator") {
            type = ActionDomain::Separator;
        } else if (name == "stretch") {
            type = ActionDomain::Stretch;
        } else if (name == "menu") {
            type = ActionDomain::Menu;
        }
        return type;
    }

    static int m_separatorCount = 0;
    static int m_stretchCount = 0;

    static void parseRecursiveMenuTree(ActionDomain *domain, const QMXmlAdaptorElement *ele, const QString &parentId) {
        for (const auto &sub_item : qAsConst(ele->children)) {
            QString id = sub_item->properties.value("id");
            if (id.isEmpty()) {
                if (sub_item->name == "separator") {
                    id = "separator_" + QString::number(++m_separatorCount);
                } else if (sub_item->name == "stretch") {
                    id = "stretch_" + QString::number(++m_stretchCount);
                } else {
                    continue;
                }
            }

            domain->addInsertRule(
                id, parentId,
                ActionInsertRule{ActionInsertRule::Append, sub_item->properties.value("expand") == "true"});

            if (!domain->containsAction(id)) {
                domain->addAction(id, parseActionDomainType(sub_item->name));
            }

            if (!sub_item->children.isEmpty()) {
                parseRecursiveMenuTree(domain, sub_item.data(), id);
            }
        }
    }

    bool ActionSystem::loadDomains(const QString &fileName) {
        Q_D(ActionSystem);

        QMXmlAdaptor file;
        if (!file.load(fileName)) {
            myWarning(__func__) << "load file failed";
            return false;
        }

        const auto &root = file.root;
        if (root.name != "actionSystem") {
            return false;
        }

        for (const auto &item : qAsConst(root.children)) {
            // Parse action items
            if (item->name == "actions" || item->name == "menus") {
                for (const auto &action_item : qAsConst(item->children)) {
                    if (action_item->name != "item") {
                        continue;
                    }

                    const auto &properties = action_item->properties;

                    // id
                    const auto &id = properties.value("id");

                    // validate id
                    if (id.isEmpty() || d->actions.contains(id))
                        continue;

                    auto spec = new ActionSpec(id);

                    // command name
                    auto it = properties.find("command");
                    if (it != properties.end()) {
                        spec->setCommandName(it.value());
                    }

                    // shortcuts
                    it = properties.find("shortcut");
                    if (it != properties.end()) {
                        spec->setShortcuts({QKeySequence(it.value())});
                    } else {
                        it = properties.find("shortcuts");
                        if (it != properties.end()) {
                            spec->setShortcuts(parseShortcuts(it.value()));
                        }
                    }

                    // icon
                    it = properties.find("icon");
                    if (it != properties.end()) {
                        const auto &iconArg = it.value();
                        if (iconArg.startsWith("svg(") && iconArg.endsWith(')')) {
                            spec->setIcon(QIcon(iconArg.mid(4, iconArg.size() - 5) + ", .svgx"));
                        } else {
                            spec->setIcon(QIcon(iconArg));
                        }
                    }

                    addAction(spec);
                }
                continue;
            }

            // Parse action domains
            if (item->name == "domains") {
                for (const auto &domain_item : qAsConst(item->children)) {
                    if (domain_item->name != "domain") {
                        continue;
                    }

                    // id
                    const auto &domainId = domain_item->properties.value("id");

                    // validate id
                    if (domainId.isEmpty())
                        continue;

                    auto domain = d->domains.value(domainId);
                    if (!domain) {
                        domain = new ActionDomain(domainId);
                        addDomain(domain);
                    }

                    for (const auto &topLevel_item : qAsConst(domain_item->children)) {
                        const auto &properties = topLevel_item->properties;

                        // top level menu
                        QString id = properties.value("id");
                        if (id.isEmpty()) {
                            continue;
                        }

                        if (topLevel_item->name == "insert") {
                            const auto &target = properties.value("target").split('/');
                            const auto &modeStr = properties.value("mode");
                            const auto &expand = properties.value("expand") == "true";

                            if (target.isEmpty() || target.front().isEmpty())
                                continue;

                            ActionInsertRule::InsertMode mode = ActionInsertRule::Append;
                            if (!modeStr.isEmpty()) {
                                if (modeStr == "prepend" || modeStr == "unshift") {
                                    mode = ActionInsertRule::Unshift;
                                }
                            }

                            domain->addInsertRule(id, target.first(), {mode, target.at(1), expand});
                            continue;
                        }

                        // insertion
                        if (id.startsWith('%')) {
                            domain->addTopLevelMenu(id.mid(1));
                        } else {
                            domain->addAction(id, parseActionDomainType(id));
                        }

                        for (const auto &sub_item : qAsConst(topLevel_item->children)) {
                            parseRecursiveMenuTree(domain, sub_item.data(), id);
                        }
                    }
                }

                continue;
            }
        }

        return true;
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