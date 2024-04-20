#include "actiondomain.h"
#include "actiondomain_p.h"

#include <utility>

#include <QFileInfo>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

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
                    << "Core::ActionCatalogue::setChildren(): duplicated child name "
                    << item.name();
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
    QList<ActionLayout> ActionLayout::children() const {
        return d->children;
    }
    void ActionLayout::addChild(const ActionLayout &child) {
        d->children.append(child);
    }
    void ActionLayout::setChildren(const QList<ActionLayout> &children) {
        d->children = children;
    }



    ActionDomainPrivate::ActionDomainPrivate() {
    }
    ActionDomainPrivate::~ActionDomainPrivate() = default;
    void ActionDomainPrivate::init() {
    }
    void ActionDomainPrivate::flushCatalogue() const {
        if (catalogue)
            return;

        struct TreeNode {
            QByteArray name;
            QString id;
            QMChronoMap<QByteArray, int> children;
            ActionCatalogue toCatalogue(const QVector<TreeNode> &heap) const {
                ActionCatalogue res;
                res.setName(name);
                res.setId(id);
                QList<ActionCatalogue> children1;
                children1.reserve(children.size());
                for (const auto &childIdx : children) {
                    children1.append(heap.at(childIdx).toCatalogue(heap));
                }
                res.setChildren(children1);
                return res;
            }
        };

        QVector<TreeNode> heap;
        TreeNode root1({}), *root = &root1;
        for (const auto &item : objectInfoMap) {
            auto p = root;
            for (const auto &c : item.categories()) {
                int idx = p->children.value(c, -1);
                if (idx >= 0) {
                    p = &heap[idx];
                    continue;
                }

                TreeNode q;
                q.name = c;
                p->children.append(c, heap.size());
                heap.append(q);
                p = &heap.back();
            }
            p->id = item.id();
        }
        catalogue = root->toCatalogue(heap);
    }

    void ActionDomainPrivate::flushLayouts() const {
        if (layouts)
            return;

        struct TreeNode {
            QString id;
            ActionObjectInfo::Type type = ActionObjectInfo::Action;
            bool flat = false;
            QVector<int> children;

            static int layoutInfoToLayout(const ActionLayoutInfo &layout,
                                          QVector<TreeNode> &layoutInstances,
                                          QHash<int, int> &extensionIndexes,
                                          QHash<QString, QVector<int>> &idIndexes) {
                if (layout.type() == ActionObjectInfo::Separator) {
                    return 0;
                }
                if (layout.type() == ActionObjectInfo::Stretch) {
                    return 1;
                }
                auto it = extensionIndexes.find(layout.idx);
                if (it != extensionIndexes.end()) {
                    return it.value();
                }

                TreeNode res;
                res.id = layout.id();
                res.type = layout.type();
                res.flat = layout.flat();

                res.children.reserve(layout.childCount());
                for (int i = 0; i < layout.childCount(); ++i) {
                    res.children.append(layoutInfoToLayout(layout.child(i), layoutInstances,
                                                           extensionIndexes, idIndexes));
                }

                int instanceIdx = layoutInstances.size();
                layoutInstances.append(res);
                extensionIndexes.insert(layout.idx, instanceIdx);
                idIndexes[layout.id()].append(instanceIdx);
                return instanceIdx;
            }

            ActionLayout toLayout(const QVector<TreeNode> &heap, int idx,
                                  QHash<int, ActionLayout> &layoutIndexes) const {
                if (auto it = layoutIndexes.find(idx); it != layoutIndexes.end()) {
                    return it.value();
                }

                ActionLayout res;
                res.setId(id);
                res.setType(type);
                res.setFlat(flat);
                QList<ActionLayout> children1;
                children1.reserve(children.size());
                for (const auto &childIdx : children) {
                    children1.append(heap.at(childIdx).toLayout(heap, childIdx, layoutIndexes));
                }
                res.setChildren(children1);
                layoutIndexes.insert(idx, res);
                return res;
            }
        };

        QVector<TreeNode> heap;
        heap.append({{}, ActionObjectInfo::Separator, false, {}});
        heap.append({{}, ActionObjectInfo::Stretch, false, {}});

        QHash<QString, QVector<int>> idIndexes;
        QHash<const ActionExtension *, QHash<int, int>> extensionIndexes;
        for (const auto &ext : extensions) {
            for (int i = 0; i < ext->layoutCount(); ++i) {
                std::ignore = TreeNode::layoutInfoToLayout(ext->layout(i), heap,
                                                           extensionIndexes[ext], idIndexes);
            }
        }

        // Apply build routines
        for (const auto &ext : extensions) {
            auto &curExtIndexes = extensionIndexes[ext];
            for (int i = 0; i < ext->buildRoutineCount(); ++i) {
                auto routine = ext->buildRoutine(i);
                auto it = idIndexes.find(routine.parent());
                if (it == idIndexes.end())
                    continue;

                QVector<int> layoutsToInsert;
                layoutsToInsert.reserve(routine.itemCount());
                for (int j = 0; j < routine.itemCount(); ++j) {
                    auto idx = TreeNode::layoutInfoToLayout(routine.item(j), heap, curExtIndexes,
                                                            idIndexes);
                    layoutsToInsert.append(idx);
                }
                for (const auto &parentIdx : std::as_const(it.value())) {
                    auto &parentLayout = heap[parentIdx];
                    switch (routine.anchor()) {
                        case ActionBuildRoutine::Last: {
                            parentLayout.children += layoutsToInsert;
                            break;
                        }
                        case ActionBuildRoutine::First: {
                            parentLayout.children = layoutsToInsert + parentLayout.children;
                            break;
                        }
                        case ActionBuildRoutine::After: {
                            for (int j = 0; j < parentLayout.children.size(); ++j) {
                                if (heap[parentLayout.children[j]].id == routine.relativeTo()) {
                                    parentLayout.children.insert(j + 1, layoutsToInsert.size(), 0);
                                    for (int k = 0; k < layoutsToInsert.size(); ++k) {
                                        parentLayout.children[j + 1 + k] = layoutsToInsert[k];
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                        case ActionBuildRoutine::Before: {
                            for (int j = 0; j < parentLayout.children.size(); ++j) {
                                if (heap[parentLayout.children[j]].id == routine.relativeTo()) {
                                    parentLayout.children.insert(j, layoutsToInsert.size(), 0);
                                    for (int k = 0; k < layoutsToInsert.size(); ++k) {
                                        parentLayout.children[j + k] = layoutsToInsert[k];
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

        // Convert to layout
        QHash<int, ActionLayout> layoutIndexes;
        QList<ActionLayout> result;
        for (int i = 0; i < heap.size(); ++i) {
            const auto &item = heap.at(i);
            auto it = objectInfoMap.find(item.id);
            if (it == objectInfoMap.end())
                continue;
            if (it->type() != ActionObjectInfo::Menu || it->shape() != ActionItem::TopLevel)
                continue;
            result.append(item.toLayout(heap, i, layoutIndexes));
        }
        layouts = result;
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
                    auto it = map.find(itemToBeChanged.theme);
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

    ActionDomain::ActionDomain(QObject *parent) : ActionDomain(*new ActionDomainPrivate(), parent) {
    }
    ActionDomain::~ActionDomain() = default;

    static QSharedPointer<QMXmlAdaptorElement>
        createLayoutElementRecursively(const ActionLayout &layout) {
        auto e = QSharedPointer<QMXmlAdaptorElement>::create();
        if (auto id = layout.id(); !id.isEmpty())
            e->properties.insert(QStringLiteral("id"), id);

        switch (layout.type()) {
            case ActionObjectInfo::Action:
                e->name = QStringLiteral("action");
                break;
            case ActionObjectInfo::Group:
                e->name = QStringLiteral("group");
                break;
            case ActionObjectInfo::Menu:
                e->name = QStringLiteral("menu");
                if (layout.flat())
                    e->properties.insert(QStringLiteral("flat"), QStringLiteral("true"));
                break;
            case ActionObjectInfo::Separator:
                e->name = QStringLiteral("separator");
                break;
            case ActionObjectInfo::Stretch:
                e->name = QStringLiteral("stretch");
                break;
        }

        for (const auto &child : layout.children())
            e->children.append(createLayoutElementRecursively(child));
        return e;
    }

    QByteArray ActionDomain::saveLayouts() const {
        Q_D(const ActionDomain);

        d->flushLayouts();

        QMXmlAdaptor xml;

        QMXmlAdaptorElement &root = xml.root;
        root.name = QStringLiteral("actionDomain");

        auto extensionsElement = QSharedPointer<QMXmlAdaptorElement>::create();
        extensionsElement->name = QStringLiteral("extensions");
        extensionsElement->children.reserve(d->extensions.size());
        for (const auto &item : d->extensions) {
            auto e = QSharedPointer<QMXmlAdaptorElement>::create();
            e->name = QStringLiteral("extension");
            e->properties.insert(QStringLiteral("hash"), item->hash());
            extensionsElement->children.append(e);
        }

        auto layoutsElement = QSharedPointer<QMXmlAdaptorElement>::create();
        layoutsElement->name = QStringLiteral("layouts");
        layoutsElement->children.reserve(d->layouts->size());
        for (const auto &item : d->layouts.value()) {
            layoutsElement->children.append(createLayoutElementRecursively(item));
        }

        root.children.append({extensionsElement, layoutsElement});

        return xml.saveData();
    }
    bool ActionDomain::restoreLayouts(const QByteArray &obj) {
        // TODO
        return false;
    }
    QByteArray ActionDomain::saveOverriddenShortcuts() const {
        Q_D(const ActionDomain);
        QJsonObject rootObj;
        for (auto it = d->overriddenShortcuts.begin(); it != d->overriddenShortcuts.end(); ++it) {
            auto it2 = d->objectInfoMap.find(it.key());
            if (it2 == d->objectInfoMap.end())
                continue;
            if (it2->type() != ActionObjectInfo::Action || it2->shape() != ActionObjectInfo::Plain)
                continue;

            QJsonValue value = QJsonValue::Null;
            if (const auto &val = it.value()) {
                QJsonArray arr;
                for (const auto &subItem : val.value()) {
                    arr.push_back(subItem.toString());
                }
                value = arr;
            }
            rootObj.insert(it.key(), value);
        }
        return QJsonDocument(rootObj).toJson();
    }
    bool ActionDomain::restoreOverriddenShortcuts(const QByteArray &obj) {
        // TODO
        return false;
    }
    QByteArray ActionDomain::saveOverriddenIcons() const {
        Q_D(const ActionDomain);
        QJsonObject rootObj;
        for (auto it = d->overriddenIcons.begin(); it != d->overriddenIcons.end(); ++it) {
            QJsonValue value = QJsonValue::Null;
            if (const auto &val = it.value()) {
                QJsonObject obj;
                obj.insert(QStringLiteral("fromFile"), val->fromFile());
                obj.insert(QStringLiteral("data"), val->data());
                value = obj;
            }
            rootObj.insert(it.key(), value);
        }
        return QJsonDocument(rootObj).toJson();
    }
    bool ActionDomain::restoreOverriddenIcons(const QByteArray &obj) {
        // TODO
        return false;
    }
    void ActionDomain::addExtension(const ActionExtension *extension) {
        Q_D(ActionDomain);

        if (d->extensions.contains(extension->hash())) {
            qWarning().noquote().nospace()
                << "Core::ActionDomain::addExtension(): duplicated extension hash "
                << extension->hash();
            return;
        }

        QHash<QString, ActionObjectInfo> objectInfoMapTemp;
        QSet<QByteArrayList> objectCategories;
        objectInfoMapTemp.reserve(extension->objectCount());

        // Check duplication
        for (int i = 0; i < extension->objectCount(); ++i) {
            auto obj = extension->object(i);
            auto id = obj.id();
            if (d->objectInfoMap.contains(id)) {
                qWarning().noquote().nospace()
                    << "Core::ActionDomain::addExtension(): duplicated object id " << id;
                return;
            }
            auto categories = obj.categories();
            if (d->objectCategories.contains(categories)) {
                qWarning().noquote().nospace()
                    << "Core::ActionDomain::addExtension(): duplicated object categories "
                    << categories;
                return;
            }
            objectInfoMapTemp.insert(id, obj);
            objectCategories.insert(categories);
        }

        for (auto it = objectInfoMapTemp.begin(); it != objectInfoMapTemp.end(); ++it) {
            d->objectInfoMap.append(it.key(), it.value());
        }
        d->objectCategories += objectCategories;

        d->extensions.append(extension->hash(), extension);
        d->catalogue.reset();
        d->layouts.reset();
    }
    void ActionDomain::removeExtension(const ActionExtension *extension) {
        Q_D(ActionDomain);
        for (int i = 0; i < extension->objectCount(); ++i) {
            auto obj = extension->object(i);
            d->objectInfoMap.remove(obj.id());
            d->objectCategories.remove(obj.categories());
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
        return d->objectInfoMap.keys_qlist();
    }
    ActionObjectInfo ActionDomain::objectInfo(const QString &objId) const {
        Q_D(const ActionDomain);
        auto it = d->objectInfoMap.find(objId);
        if (it == d->objectInfoMap.end())
            return {};
        return it.value();
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
    std::optional<ActionDomain::IconReference>
        ActionDomain::overriddenIcon(const QString &objId) const {
        Q_D(const ActionDomain);
        return d->overriddenIcons.value(objId);
    }
    void ActionDomain::setOverriddenIcon(const QString &objId,
                                         const std::optional<IconReference> &iconRef) {
        Q_D(ActionDomain);
        if (iconRef && iconRef->fromFile()) {
            QFileInfo info(iconRef->data());
            if (!info.isFile())
                return;
        }
        d->overriddenIcons.insert(objId, iconRef);
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
