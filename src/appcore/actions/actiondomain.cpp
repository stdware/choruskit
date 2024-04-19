#include "actiondomain.h"
#include "actiondomain_p.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <utility>

#include <qmxmladaptor.h>

namespace Core {

    static QString parseExpression(QString s, const QHash<QString, QString> &vars) {
        QRegularExpressionMatch match;
        int index = 0;
        bool hasMatch = false;

        static QRegularExpression reg(QStringLiteral(R"(\$\{(\w+)\})"));
        while ((index = s.indexOf(reg, index, &match)) != -1) {
            hasMatch = true;

            const auto &name = match.captured(1);
            QString val;
            auto it = vars.find(name);
            if (it == vars.end()) {
                val = name;
            } else {
                val = it.value();
            }

            s.replace(index, match.captured(0).size(), val);
        }
        if (!hasMatch) {
            return s;
        }
        return parseExpression(s, vars);
    }

    class IconConfigParser {
    public:
        struct ParserConfig {
            QString baseDirectory;
        };

        inline QString resolve(const QString &s) const {
            return parseExpression(s, variables);
        }

        QHash<QString, QHash<QString, QString>> parse() {
            QMXmlAdaptor xml;
            if (!xml.load(fileName)) {
                fprintf(stdout, "Core::ActionDomain():%s: failed to read icon configuration file\n",
                        qPrintable(fileName));
                return {};
            }

            const auto &root = xml.root;
            if (const auto &rootName = root.name; rootName != QStringLiteral("iconConfiguration")) {
                fprintf(stdout, "Core::ActionDomain():%s: unknown root element tag %s\n",
                        qPrintable(fileName), rootName.toLatin1().data());
                return {};
            }
            bool hasParserConfig = false;
            ParserConfig parserConfig;
            parserConfig.baseDirectory = QFileInfo(fileName).absolutePath();

            QList<const QMXmlAdaptorElement *> iconsElements;
            for (const auto &item : std::as_const(root.children)) {
                if (item->name == QStringLiteral("icons")) {
                    iconsElements.append(item.data());
                    continue;
                }
                if (item->name == QStringLiteral("parserConfig")) {
                    if (hasParserConfig) {
                        fprintf(stdout,
                                "Core::ActionDomain():%s: duplicated parser config elements\n",
                                qPrintable(fileName));
                        return {};
                    }
                    parserConfig = parseParserConfig(*item);
                    hasParserConfig = true;
                    continue;
                }
            }

            // Get results
            QHash<QString, QHash<QString, QString>> result;
            for (const auto &item : std::as_const(iconsElements)) {
                auto theme = item->properties.value(QStringLiteral("theme"));
                std::list<const QMXmlAdaptorElement *> stack;
                for (const auto &child : item->children) {
                    stack.push_back(child.data());
                }

                auto &themeMap = result[theme];
                while (!stack.empty()) {
                    auto e = stack.front();
                    stack.pop_front();

                    auto id = resolve(e->properties.value(QStringLiteral("id")));
                    if (!id.isEmpty()) {
                        auto file = resolve(e->properties.value(QStringLiteral("file")));
                        if (!file.isEmpty()) {
                            file.replace(QStringLiteral(":/"), parserConfig.baseDirectory);
                            themeMap[id] = file;
                        }
                    }

                    for (const auto &child : item->children) {
                        stack.push_back(child.data());
                    }
                }
            }
            return result;
        }

        ParserConfig parseParserConfig(const QMXmlAdaptorElement &e) {
            ParserConfig result;

            for (const auto &item : e.children) {
                if (item->name == QStringLiteral("baseDirectory")) {
                    result.baseDirectory = resolve(item->value);
                    continue;
                }

                if (item->name == QStringLiteral("vars")) {
                    for (const auto &subItem : item->children) {
                        auto key = resolve(subItem->properties.value(QStringLiteral("key")));
                        auto value = resolve(subItem->properties.value(QStringLiteral("value")));
                        if (!key.isEmpty()) {
                            variables.insert(key, value);
                        }
                    }
                }
            }
            return result;
        }

    public:
        QString fileName;
        QHash<QString, QString> variables;
    };

    ActionCatalogue::ActionCatalogue() : d(new ActionCatalogueData()) {
    }
    ActionCatalogue::ActionCatalogue(const QByteArray &name) : ActionCatalogue() {
        d->name = name;
    }
    ActionCatalogue::ActionCatalogue(const ActionCatalogue &other) = default;
    ActionCatalogue &ActionCatalogue::operator=(const ActionCatalogue &other) = default;
    ActionCatalogue::~ActionCatalogue() = default;
    QByteArray ActionCatalogue::name() const {
        return d->name;
    }
    void ActionCatalogue::setName(const QByteArray &name) {
        d->name = name;
    }
    QString ActionCatalogue::id() const {
        return d->id;
    }
    void ActionCatalogue::setId(const QString &id) {
        d->id = id;
    }
    QList<ActionCatalogue> ActionCatalogue::children() const {
        return d->children;
    }
    void ActionCatalogue::setChildren(const QList<ActionCatalogue> &children) {
        QHash<QByteArray, int> indexes;
        indexes.reserve(children.size());
        for (int i = 0; i < children.size(); ++i) {
            const auto &item = children.at(i);
            if (indexes.contains(item.name())) {
                qWarning().noquote().nospace()
                    << "Core::ActionCatalogue::setChildren(): duplicated child name " << item.name();
                return;
            }
            indexes.insert(item.name(), i);
        }
        d->children = children;
        d->indexes = indexes;
    }
    int ActionCatalogue::indexOfChild(const QByteArray &name) const {
        return d->indexes.value(name, -1);
    }



    ActionLayout::ActionLayout() : d(new ActionLayoutData()) {
    }
    ActionLayout::ActionLayout(const QString &id) : ActionLayout() {
        d->id = id;
    }
    ActionLayout::ActionLayout(const ActionLayout &other) = default;
    ActionLayout &ActionLayout::operator=(const ActionLayout &other) = default;
    ActionLayout::~ActionLayout() = default;
    QString ActionLayout::id() const {
        return d->id;
    }
    void ActionLayout::setId(const QString &id) {
        d->id = id;
    }
    ActionObjectInfo::Type ActionLayout::type() const {
        return d->type;
    }
    void ActionLayout::setType(ActionObjectInfo::Type type) {
        d->type = type;
    }
    bool ActionLayout::flat() const {
        return d->flat;
    }
    void ActionLayout::setFlat(bool flat) {
        d->flat = flat;
    }
    ActionLayout::IconReference ActionLayout::iconReference() const {
        return d->icon;
    }
    void ActionLayout::setIconReference(const IconReference &icon) {
        d->icon = icon;
    }
    QList<ActionLayout> ActionLayout::children() const {
        return d->children;
    }
    void ActionLayout::setChildren(const QList<ActionLayout> &children) {
        QHash<QString, int> indexes;
        indexes.reserve(children.size());
        for (int i = 0; i < children.size(); ++i) {
            const auto &item = children.at(i);
            if (indexes.contains(item.id())) {
                qWarning().noquote().nospace()
                    << "Core::ActionCatalogue::setChildren(): duplicated child id " << item.id();
                return;
            }
            indexes.insert(item.id(), i);
        }
        d->children = children;
        d->indexes = indexes;
    }
    int ActionLayout::indexOfChild(const QString &id) const {
        return d->indexes.value(id, -1);
    }



    ActionDomainPrivate::ActionDomainPrivate() {
    }

    ActionDomainPrivate::~ActionDomainPrivate() {
    }

    void ActionDomainPrivate::init() {
    }

    void ActionDomainPrivate::flushCatalogue() const {
        if (catalogue)
            return;

        struct TreeNode {
            QByteArray name;
            QString id;
            QMChronoMap<QByteArray, TreeNode *> children;
            explicit TreeNode(QByteArray name) : name(std::move(name)) {
            }
            ~TreeNode() {
                qDeleteAll(children);
            }

            ActionCatalogue toCatalogue() const {
                ActionCatalogue res;
                res.setName(name);
                res.setId(id);
                QList<ActionCatalogue> children1;
                children1.reserve(children.size());
                for (const auto &item : children) {
                    children1.append(item->toCatalogue());
                }
                return res;
            }
        };

        TreeNode root1({}), *root = &root1;
        for (const auto &item : objectInfoMap) {
            auto p = root;
            for (const auto &c : item->categories) {
                auto it = p->children.find(c);
                if (it != p->children.end()) {
                    p = it.value();
                    continue;
                }

                auto q = new TreeNode(c);
                p->children.append(c, q);
                p = q;
            }
            p->id = item->id;
        }
        catalogue = root->toCatalogue();
    }
    void ActionDomainPrivate::flushLayouts() const {
        if (layouts)
            return;


    }
    void ActionDomainPrivate::flushIcons() const {
        auto &changes = iconChange.items;
        if (changes.isEmpty())
            return;

        for (const auto &c : std::as_const(changes)) {
            if (c.index() == 0) {
                auto &map = iconStorage.singles;
                auto &indexes = iconStorage.items;
                auto itemToBeChanged = std::get<0>(c);
                if (itemToBeChanged.remove) {
                    auto it = map.find({itemToBeChanged.theme});
                    if (it != map.end()) {
                        auto &map0 = it.value();
                        if (map0.remove(itemToBeChanged.id)) {
                            if (map0.isEmpty()) {
                                map.erase(it);
                            }
                            indexes.remove({itemToBeChanged.theme, itemToBeChanged.id});
                        }
                    }
                } else if (auto info = QFileInfo(itemToBeChanged.fileName); info.isFile()) {
                    QStringList keys = {itemToBeChanged.theme, itemToBeChanged.id};
                    map[itemToBeChanged.theme][itemToBeChanged.id] = info.canonicalFilePath();
                    indexes.remove(keys);
                    indexes.append(keys);
                }
            } else {
                auto &map = iconStorage.configFiles;
                auto &indexes = iconStorage.items;
                auto itemToBeChanged = std::get<1>(c);
                if (itemToBeChanged.remove) {
                    if (map.remove(itemToBeChanged.fileName)) {
                        indexes.remove({itemToBeChanged.fileName});
                    }
                } else if (auto iconsFromFile =
                               IconConfigParser{itemToBeChanged.fileName, {}}.parse();
                           !iconsFromFile.isEmpty()) {
                    QStringList keys = {itemToBeChanged.fileName};
                    map[itemToBeChanged.fileName] = {};
                    indexes.remove(keys);
                    indexes.append(keys);
                }
            }
        }
        changes.clear();

        // Build map
        auto &map = iconStorage.storage;
        map.clear();
        for (const auto &keys : std::as_const(iconStorage.items)) {
            if (keys.size() == 1) {
                auto configMap = iconStorage.configFiles.value(keys[0]);
                for (auto it = configMap.begin(); it != configMap.end(); ++it) {
                    auto &from = it.value();
                    auto &to = map[it.key()];
                    for (auto it2 = from.begin(); it2 != from.end(); ++it2) {
                        to.insert(it2.key(), it2.value());
                    }
                }
            } else {
                map[keys[0]][keys[1]] = iconStorage.singles.value(keys[0]).value(keys[1]);
            }
        }
    }

    ActionDomain::ActionDomain(QObject *parent) : QObject(parent) {
    }
    ActionDomain::~ActionDomain() = default;
    QByteArray ActionDomain::saveLayouts() const {
        // TODO
        return {};
    }
    bool ActionDomain::restoreLayouts(const QByteArray &obj) {
        // TODO
        return false;
    }
    QByteArray ActionDomain::saveOverriddenAttributes() const {
        // TODO
        return {};
    }
    bool ActionDomain::restoreOverriddenAttributes(const QByteArray &obj) {
        // TODO
        return false;
    }
    void ActionDomain::addExtension(const ActionExtension *extension) {
        Q_D(ActionDomain);

        QHash<QString, const ActionObjectInfoData *> objectInfoMapTemp;
        objectInfoMapTemp.reserve(extension->objectCount());

        // Check duplication
        for (int i = 0; i < extension->objectCount(); ++i) {
            auto objData = static_cast<const ActionObjectInfoData *>(extension->object(i).data);
            if (d->objectInfoMap.contains(objData->id)) {
                qWarning().noquote().nospace()
                    << "Core::ActionDomain::addExtension(): duplicated object id " << objData->id;
                return;
            }
            objectInfoMapTemp.insert(objData->id, objData);
        }

        if (d->extensions.contains(extension->hash())) {
            qWarning().noquote().nospace()
                << "Core::ActionDomain::addExtension(): duplicated extension hash "
                << extension->hash();
            return;
        }

        for (auto it = objectInfoMapTemp.begin(); it != objectInfoMapTemp.end(); ++it) {
            d->objectInfoMap.append(it.key(), it.value());
        }
        d->extensions.append(extension->hash(), extension);
        d->catalogue.reset();
        d->layouts.reset();
    }
    void ActionDomain::removeExtension(const ActionExtension *extension) {
        Q_D(ActionDomain);
        for (int i = 0; i < extension->objectCount(); ++i) {
            d->objectInfoMap.remove(extension->object(i).id());
        }
        d->extensions.remove(extension->hash());
        d->catalogue.reset();
        d->layouts.reset();
    }
    void ActionDomain::addIcon(const QString &theme, const QString &id, const QString &fileName) {
        Q_D(ActionDomain);
        if (!QFileInfo(fileName).isFile()) {
            return;
        }

        ActionDomainPrivate::IconChange::Single itemToBeAdded{
            theme,
            id,
            fileName,
            false,
        };
        auto &items = d->iconChange.items;
        QStringList keys = {theme, id};
        items.remove(keys);
        items.append(keys, {itemToBeAdded});
    }
    void ActionDomain::addIconConfiguration(const QString &fileName) {
        Q_D(ActionDomain);
        if (!QFileInfo(fileName).isFile()) {
            return;
        }

        ActionDomainPrivate::IconChange::Config itemToBeAdded{
            fileName,
            false,
        };
        auto &items = d->iconChange.items;
        QStringList keys = {fileName};
        items.remove(keys);
        items.append({fileName}, {itemToBeAdded});
    }
    void ActionDomain::removeIcon(const QString &theme, const QString &id) {
        Q_D(ActionDomain);
        auto &items = d->iconChange.items;
        ActionDomainPrivate::IconChange::Single itemToBeRemoved{
            theme,
            id,
            {},
            true,
        };
        QStringList keys = {theme, id};
        items.remove(keys);
        items.append(keys, itemToBeRemoved);
    }
    void ActionDomain::removeIconConfiguration(const QString &fileName) {
        Q_D(ActionDomain);
        auto &items = d->iconChange.items;
        ActionDomainPrivate::IconChange::Config itemToBeRemoved{
            fileName,
            true,
        };
        QStringList keys = {fileName};
        items.remove(keys);
        items.append({fileName}, itemToBeRemoved);
    }
    QStringList ActionDomain::objectIds() const {
        Q_D(const ActionDomain);
        auto arr = d->objectInfoMap.keys();
        return {arr.begin(), arr.end()};
    }
    ActionObjectInfo ActionDomain::objectInfo(const QString &objId) const {
        Q_D(const ActionDomain);
        auto it = d->objectInfoMap.find(objId);
        if (it == d->objectInfoMap.end())
            return {};
        ActionObjectInfo res;
        res.data = it.value();
        return res;
    }
    ActionCatalogue ActionDomain::catalogue() const {
        Q_D(const ActionDomain);
        d->flushCatalogue();
        return d->catalogue.value();
    }
    QStringList ActionDomain::iconThemes() const {
        Q_D(const ActionDomain);
        d->flushIcons();
        return d->iconStorage.storage.keys();
    }
    QStringList ActionDomain::iconIds(const QString &theme) {
        Q_D(const ActionDomain);
        d->flushIcons();
        return d->iconStorage.storage.value(theme).keys();
    }
    QIcon ActionDomain::icon(const QString &theme, const QString &iconId) const {
        Q_D(const ActionDomain);
        d->flushIcons();
        auto icon = QIcon(d->iconStorage.storage.value(theme).value(iconId));
        if (!icon.isNull())
            return {};
        return QIcon(d->iconStorage.storage.value({}).value(iconId)); // fallback
    }
    QList<ActionLayout> ActionDomain::layouts() const {
        Q_D(const ActionDomain);
        d->flushLayouts();
        return d->layouts.value();
    }
    void ActionDomain::setLayouts(const QList<ActionLayout> &layouts) {
        Q_D(ActionDomain);
        d->layouts = layouts;
    }
    void ActionDomain::resetLayouts() {
        Q_D(ActionDomain);
        d->layouts.reset();
        d->flushLayouts();
    }
    std::optional<QList<QKeySequence>>
        ActionDomain::overriddenShortcuts(const QString &objId) const {
        Q_D(const ActionDomain);
        return d->overriddenShortcuts.value(objId);
    }
    void ActionDomain::setOverriddenShortcuts(const QString &objId,
                                              const std::optional<QList<QKeySequence>> &shortcuts) {
        Q_D(ActionDomain);
        d->overriddenShortcuts.insert(objId, shortcuts);
    }
    void ActionDomain::resetShortcuts() {
        Q_D(ActionDomain);
        d->overriddenShortcuts.clear();
    }
    std::optional<QString> ActionDomain::overriddenIconFile(const QString &objId) const {
        Q_D(const ActionDomain);
        return d->overriddenIcons.value(objId);
    }
    void ActionDomain::setOverriddenIconFile(const QString &objId,
                                             const std::optional<QString> &fileName) {
        Q_D(ActionDomain);
        if (fileName) {
            QFileInfo info(fileName.value());
            if (!info.isFile())
                return;
        }
        d->overriddenIcons.insert(objId, fileName);
    }
    void ActionDomain::resetIconFiles() {
        Q_D(ActionDomain);
        d->overriddenIcons.clear();
    }
    bool ActionDomain::buildLayouts(const QString &theme, const QList<ActionItem *> &items) const {
        Q_D(const ActionDomain);
        d->flushLayouts();

        // TODO

        return false;
    }

    ActionDomain::ActionDomain(ActionDomainPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;

        d.init();
    }

}
