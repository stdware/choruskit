#include "actiondomain.h"
#include "actiondomain_p.h"

#include <utility>

#include <QFileInfo>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMetaProperty>

#include <qmxmladaptor.h>

#include "actionitem_p.h"

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
                fprintf(stdout, "Core::ActionDomain: %s: failed to read icon configuration file\n",
                        qPrintable(fileName));
                return {};
            }

            const auto &root = xml.root;
            if (const auto &rootName = root.name; rootName != QStringLiteral("iconConfiguration")) {
                fprintf(stdout, "Core::ActionDomain: %s: unknown root element tag \"%s\"\n",
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
                                "Core::ActionDomain: %s: duplicated parser config elements\n",
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
    ActionLayoutInfo::Type ActionLayout::type() const {
        return d->type;
    }
    void ActionLayout::setType(ActionLayoutInfo::Type type) {
        d->type = type;
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



    ActionDomainPrivate::ActionDomainPrivate()
        : sharedStretchWidgetAction(new StretchWidgetAction()),
          sharedMenuItem(new ActionItem(
              {}, ActionItem::MenuFactory([](QWidget *parent) { return new QMenu(parent); }))) {
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

    class LayoutsHelper {
    private:
        struct TreeNode {
            QString id;
            ActionLayoutInfo::Type type = ActionLayoutInfo::Action;
            QVector<int> children;

            inline TreeNode(ActionLayoutInfo::Type type = ActionLayoutInfo::Action) : type(type) {
            }

            static int layoutInfoToLayout(const ActionLayoutInfo &layout, QVector<TreeNode> &heap,
                                          QHash<QString, QVector<int>> &idIndexes) {
                TreeNode node;
                node.type = layout.type();
                if (node.type == ActionLayoutInfo::Separator ||
                    node.type == ActionLayoutInfo::Stretch) {
                    int instanceIdx = heap.size();
                    heap.append(node);
                    return instanceIdx;
                }

                node.id = layout.id();
                node.children.reserve(layout.childCount());
                for (int i = 0; i < layout.childCount(); ++i) {
                    node.children.append(layoutInfoToLayout(layout.child(i), heap, idIndexes));
                }

                int instanceIdx = heap.size();
                heap.append(node);
                idIndexes[node.id].append(instanceIdx);
                return instanceIdx;
            }

            ActionLayout toLayout(const QVector<TreeNode> &heap,
                                  const ActionLayout &sharedSeparator,
                                  const ActionLayout &sharedStretch) const {
                if (type == ActionLayoutInfo::Separator)
                    return sharedSeparator;
                if (type == ActionLayoutInfo::Stretch)
                    return sharedStretch;
                ActionLayout layout;
                layout.setId(id);
                layout.setType(type);
                QList<ActionLayout> children1;
                children1.reserve(children.size());
                for (const auto &childIdx : children) {
                    children1.append(
                        heap.at(childIdx).toLayout(heap, sharedSeparator, sharedStretch));
                }
                layout.setChildren(children1);
                return layout;
            }
        };

        const QMChronoMap<QString, const ActionExtension *> &extensions;
        const QMChronoMap<QString, ActionObjectInfo> &objectInfoMap;

        static QSharedPointer<QMXmlAdaptorElement> serializeLayout(const ActionLayout &layout) {
            auto e = QSharedPointer<QMXmlAdaptorElement>::create();
            if (auto id = layout.id(); !id.isEmpty())
                e->properties.insert(QStringLiteral("id"), id);

            switch (layout.type()) {
                case ActionLayoutInfo::Action:
                    e->name = QStringLiteral("action");
                    break;
                case ActionLayoutInfo::Group:
                    e->name = QStringLiteral("group");
                    break;
                case ActionLayoutInfo::Menu:
                    e->name = QStringLiteral("menu");
                    break;
                case ActionLayoutInfo::ExpandedMenu:
                    e->name = QStringLiteral("menu");
                    e->properties.insert(QStringLiteral("flat"), QStringLiteral("true"));
                    break;
                case ActionLayoutInfo::Separator:
                    e->name = QStringLiteral("separator");
                    break;
                case ActionLayoutInfo::Stretch:
                    e->name = QStringLiteral("stretch");
                    break;
            }
            for (const auto &child : layout.children())
                e->children.append(serializeLayout(child));
            return e;
        }

        bool fromNodeElement(const QMXmlAdaptorElement *e, TreeNode &node) const {
            if (e->name == QStringLiteral("separator")) {
                node.type = ActionLayoutInfo::Separator;
                return true;
            }
            if (e->name == QStringLiteral("stretch")) {
                node.type = ActionLayoutInfo::Stretch;
                return true;
            }

            auto id = e->properties.value(QStringLiteral("id"));
            if (id.isEmpty()) {
                return false;
            }

            auto it = objectInfoMap.find(id);
            if (it == objectInfoMap.end())
                return false;

            node.id = id;
            switch (it->type()) {
                case ActionObjectInfo::Action:
                    node.type = ActionLayoutInfo::Action;
                    break;
                case ActionObjectInfo::Group:
                    node.type = ActionLayoutInfo::Group;
                    break;
                case ActionObjectInfo::Menu:
                    node.type =
                        e->properties.value(QStringLiteral("flat")) == QStringLiteral("true")
                            ? ActionLayoutInfo::ExpandedMenu
                            : ActionLayoutInfo::Menu;
                    break;
            }
            return true;
        };

        int restoreElementHelper(const QMXmlAdaptorElement *e, QVector<TreeNode> &heap,
                                 QHash<QString, QVector<int>> &idIndexes) const {
            TreeNode node;
            if (!fromNodeElement(e, node)) {
                return -1;
            }

            if (node.type == ActionLayoutInfo::Separator ||
                node.type == ActionLayoutInfo::Stretch) {
                int instanceIdx = heap.size();
                heap.append(node);
                return instanceIdx;
            }

            for (const auto &child : e->children) {
                auto childIdx = restoreElementHelper(child.data(), heap, idIndexes);
                if (childIdx < 0) {
                    continue;
                }
                node.children.append(childIdx);
            }

            auto instanceIdx = heap.size();
            heap.append(node);
            idIndexes[node.id].append(instanceIdx);
            return instanceIdx;
        }

    public:
        LayoutsHelper(const QMChronoMap<QString, const ActionExtension *> &extensions,
                      const QMChronoMap<QString, ActionObjectInfo> &objectInfoMap)
            : extensions(extensions), objectInfoMap(objectInfoMap) {
        }

        inline QList<ActionLayout> build() const {
            QVector<TreeNode> heap;
            QHash<QString, QVector<int>> idIndexes;
            for (const auto &ext : extensions) {
                for (int i = 0; i < ext->layoutCount(); ++i) {
                    std::ignore = TreeNode::layoutInfoToLayout(ext->layout(i), heap, idIndexes);
                }
            }
            return applyBuildRoutines(extensions.values_qlist(), heap, idIndexes);
        }

        inline QList<ActionLayout> restore(const QByteArray &data, bool *ok) const {
            QVector<TreeNode> heap;
            QHash<QString, QVector<int>> idIndexes;
            QSet<QString> extensionHashSet;

            *ok = false;
            {
                QMXmlAdaptor xml;

                // Read file
                if (!xml.loadData(data)) {
                    fprintf(stdout, "Core::ActionCatalogue::restoreLayouts(): invalid format\n");
                    return {};
                }

                // Check root name
                const auto &root = xml.root;
                if (const auto &rootName = root.name; rootName != QStringLiteral("actionDomain")) {
                    fprintf(
                        stdout,
                        "Core::ActionCatalogue::restoreLayouts(): unknown root element tag \"%s\"\n",
                        rootName.toLatin1().data());
                    return {};
                }

                for (const auto &rootChild : root.children) {
                    if (rootChild->name == QStringLiteral("extensions")) {
                        for (const auto &extensionsChild : rootChild->children) {
                            auto hash = extensionsChild->properties.value(QStringLiteral("hash"));
                            if (!hash.isEmpty())
                                extensionHashSet.insert(hash);
                        }
                        continue;
                    }

                    if (rootChild->name == QStringLiteral("layouts")) {
                        for (const auto &item : rootChild->children) {
                            std::ignore = restoreElementHelper(item.data(), heap, idIndexes);
                        }
                    }
                }
            }
            *ok = true;

            QList<const ActionExtension *> effectiveExtensions;
            for (const auto &ext : extensions) {
                if (extensionHashSet.contains(ext->hash()))
                    continue;
                effectiveExtensions.append(ext);
            }

            return applyBuildRoutines(effectiveExtensions, heap, idIndexes);
        }

        static inline QByteArray
            serialize(const QMChronoMap<QString, const ActionExtension *> &extensions,
                      const QList<ActionLayout> &layouts) {
            QMXmlAdaptor xml;

            QMXmlAdaptorElement &root = xml.root;
            root.name = QStringLiteral("actionDomain");

            auto extensionsElement = QSharedPointer<QMXmlAdaptorElement>::create();
            extensionsElement->name = QStringLiteral("extensions");
            extensionsElement->children.reserve(extensions.size());
            for (const auto &item : extensions) {
                auto e = QSharedPointer<QMXmlAdaptorElement>::create();
                e->name = QStringLiteral("extension");
                e->properties.insert(QStringLiteral("hash"), item->hash());
                extensionsElement->children.append(e);
            }

            auto layoutsElement = QSharedPointer<QMXmlAdaptorElement>::create();
            layoutsElement->name = QStringLiteral("layouts");
            layoutsElement->children.reserve(layouts.size());
            for (const auto &item : layouts) {
                layoutsElement->children.append(serializeLayout(item));
            }

            root.children.append({extensionsElement, layoutsElement});
            return xml.saveData();
        }

    private:
        QList<ActionLayout>
            applyBuildRoutines(const QList<const ActionExtension *> &effectiveExtensions,
                               QVector<TreeNode> &heap,
                               QHash<QString, QVector<int>> &idIndexes) const {
            // Apply build routines
            for (const auto &ext : effectiveExtensions) {
                for (int i = 0; i < ext->buildRoutineCount(); ++i) {
                    auto routine = ext->buildRoutine(i);
                    auto it = idIndexes.find(routine.parent());
                    if (it == idIndexes.end())
                        continue;

                    QVector<int> layoutsToInsert;
                    layoutsToInsert.reserve(routine.itemCount());
                    for (int j = 0; j < routine.itemCount(); ++j) {
                        auto idx = TreeNode::layoutInfoToLayout(routine.item(j), heap, idIndexes);
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
                                        parentLayout.children.insert(j + 1, layoutsToInsert.size(),
                                                                     0);
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
            ActionLayout sharedSeparator;
            sharedSeparator.setType(ActionLayoutInfo::Separator);
            ActionLayout sharedStretch;
            sharedStretch.setType(ActionLayoutInfo::Stretch);

            QList<ActionLayout> result;
            for (int i = 0; i < heap.size(); ++i) {
                const auto &item = heap.at(i);
                auto it = objectInfoMap.find(item.id);
                if (it == objectInfoMap.end())
                    continue;
                if (it->type() != ActionObjectInfo::Menu ||
                    it->shape() != ActionObjectInfo::TopLevel)
                    continue;
                result.append(item.toLayout(heap, sharedSeparator, sharedStretch));
            }
            return result;
        }
    };

    void ActionDomainPrivate::flushLayouts() const {
        if (layouts)
            return;
        layouts = LayoutsHelper(extensions, objectInfoMap).build();
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

    QByteArray ActionDomain::saveLayouts() const {
        Q_D(const ActionDomain);
        d->flushLayouts();
        return LayoutsHelper::serialize(d->extensions, d->layouts.value());
    }
    bool ActionDomain::restoreLayouts(const QByteArray &data) {
        Q_D(ActionDomain);

        bool ok;
        QList<ActionLayout> layouts =
            LayoutsHelper(d->extensions, d->objectInfoMap).restore(data, &ok);
        if (!ok)
            return false;

        d->layouts = layouts;
        return true;
    }
    QJsonObject ActionDomain::saveOverriddenShortcuts() const {
        Q_D(const ActionDomain);
        QJsonObject rootObj;
        for (auto it = d->overriddenShortcuts.begin(); it != d->overriddenShortcuts.end(); ++it) {
            const auto &key = it.key();

            auto it2 = d->objectInfoMap.find(key);
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
            rootObj.insert(key, value);
        }
        return rootObj;
    }
    bool ActionDomain::restoreOverriddenShortcuts(const QJsonObject &object) {
        Q_D(ActionDomain);
        QHash<QString, std::optional<QList<QKeySequence>>> result;
        for (auto it = object.begin(); it != object.end(); ++it) {
            const auto &key = it.key();
            auto it2 = d->objectInfoMap.find(key);
            if (it2 == d->objectInfoMap.end())
                continue;
            if (it2->type() != ActionObjectInfo::Action || it2->shape() != ActionObjectInfo::Plain)
                continue;

            const auto &value = it.value();
            if (value.isNull()) {
                result.insert(key, {});
                continue;
            }
            auto arr = value.toObject();
            QList<QKeySequence> shortcuts;
            shortcuts.reserve(arr.size());
            for (const auto &item : std::as_const(arr)) {
                QKeySequence ks = QKeySequence::fromString(item.toString());
                if (!ks.isEmpty()) {
                    shortcuts.append(ks);
                }
            }
            result.insert(key, shortcuts);
        }
        return false;
    }
    QJsonObject ActionDomain::saveOverriddenIcons() const {
        Q_D(const ActionDomain);
        QJsonObject rootObj;
        for (auto it = d->overriddenIcons.begin(); it != d->overriddenIcons.end(); ++it) {
            const auto &key = it.key();
            QJsonValue value = QJsonValue::Null;
            if (const auto &val = it.value()) {
                QJsonObject obj;
                obj.insert(QStringLiteral("fromFile"), val->fromFile());
                obj.insert(QStringLiteral("data"), val->data());
                value = obj;
            }
            rootObj.insert(key, value);
        }
        return rootObj;
    }
    bool ActionDomain::restoreOverriddenIcons(const QJsonObject &object) {
        Q_D(ActionDomain);
        QHash<QString, std::optional<IconReference>> result;
        for (auto it = object.begin(); it != object.end(); ++it) {
            const auto &key = it.key();
            const auto &value = it.value();
            if (value.isNull()) {
                result.insert(key, {});
                continue;
            }
            auto obj = value.toObject();
            result.insert(key, IconReference{
                                   obj.value(QStringLiteral("data")).toString(),
                                   obj.value(QStringLiteral("fromFile")).toBool(),
                               });
        }
        d->overriddenIcons = result;
        return true;
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

    class UILayoutBuildHelper {
    public:
        enum LastMenuItem {
            Action,
            Separator,
            Stretch,
        };
        QHash<QWidget *, LastMenuItem> lastMenuItems;
        const ActionDomainPrivate *d;

        UILayoutBuildHelper(const ActionDomainPrivate *d) : d(d) {
        }

        void build(const ActionLayout &layout,
                   const QHash<QString, QPair<ActionItem *, ActionObjectInfo>> &itemMap,
                   QWidget *parent) {
            switch (layout.type()) {
                case ActionLayoutInfo::Action: {
                    if (!parent)
                        break;
                    auto pair = itemMap.value(layout.id());
                    auto actionItem = pair.first;
                    if (!actionItem || pair.second.type() != ActionObjectInfo::Action) {
                        break;
                    }
                    if (actionItem->isAction()) {
                        parent->addAction(actionItem->action());
                        lastMenuItems[parent] = Action;
                    } else if (actionItem->isWidget()) {
                        parent->addAction(actionItem->widgetAction());
                        lastMenuItems[parent] = Action;
                    }
                    break;
                }
                case ActionLayoutInfo::ExpandedMenu: {
                    if (!parent)
                        break;
                    auto info = d->objectInfoMap.value(layout.id());
                    if (info.type() != ActionObjectInfo::Menu)
                        break;
                    for (const auto &childLayoutItem : layout.children()) {
                        build(childLayoutItem, itemMap, parent);
                    }
                    break;
                }
                case ActionLayoutInfo::Group: {
                    if (!parent)
                        break;
                    auto info = d->objectInfoMap.value(layout.id());
                    if (info.type() != ActionObjectInfo::Group)
                        break;
                    for (const auto &childLayoutItem : layout.children()) {
                        build(childLayoutItem, itemMap, parent);
                    }
                    break;
                }
                case ActionLayoutInfo::Menu: {
                    auto pair = itemMap.value(layout.id());
                    QWidget *nextParent;
                    auto actionItem = pair.first;
                    if (!actionItem) {
                        if (!parent)
                            break;
                        auto menu = d->sharedMenuItem->requestMenu(parent);
                        if (!menu)
                            break;
                        menu->setProperty("action-item-id", layout.id());
                        d->sharedMenuItem->addMenuAsRequested(menu);
                        parent->addAction(menu->menuAction());
                        lastMenuItems[parent] = Action;
                        nextParent = menu;
                    } else {
                        if (pair.second.type() != ActionObjectInfo::Menu)
                            break;
                        if (actionItem->isTopLevel()) {
                            auto w = actionItem->topLevel();
                            if (parent) {
                                auto menu = qobject_cast<QMenu *>(w);
                                if (menu) {
                                    parent->addAction(menu->menuAction());
                                    lastMenuItems[parent] = Action;
                                }
                            }
                            // other menu types will be ignored
                            nextParent = w;
                        } else if (actionItem->isMenu()) {
                            if (!parent)
                                break;
                            auto menu = actionItem->requestMenu(parent);
                            if (!menu) {
                                menu = d->sharedMenuItem->requestMenu(parent);
                                if (!menu)
                                    break;
                            }
                            parent->addAction(menu->menuAction());
                            lastMenuItems[parent] = Action;
                            nextParent = menu;
                        } else {
                            break;
                        }
                    }
                    for (const auto &childLayoutItem : layout.children()) {
                        build(childLayoutItem, itemMap, nextParent);
                    }
                    break;
                }
                case ActionLayoutInfo::Separator: {
                    if (lastMenuItems.value(parent) == Action) {
                        auto action = new QAction(parent);
                        action->setSeparator(true);
                        parent->addAction(action);
                        lastMenuItems[parent] = Separator;
                    }
                    break;
                }
                case ActionLayoutInfo::Stretch: {
                    if (lastMenuItems.value(parent) == Action) {
                        parent->addAction(d->sharedStretchWidgetAction.data());
                        lastMenuItems[parent] = Stretch;
                    } else if (lastMenuItems.value(parent) == Separator) {
                        parent->removeAction(parent->actions().back());
                        parent->addAction(d->sharedStretchWidgetAction.data());
                        lastMenuItems[parent] = Stretch;
                    }
                    break;
                }
            }
        }
    };

    bool ActionDomain::buildLayouts(const QList<ActionItem *> &items,
                                    const ActionItem::MenuFactory &defaultMenuFactory) const {
        Q_D(const ActionDomain);
        d->flushLayouts();

        // Build item map
        QHash<QString, QPair<ActionItem *, ActionObjectInfo>> itemMap;
        itemMap.reserve(items.size());
        for (const auto &item : items) {
            auto id = item->id();
            if (itemMap.contains(id)) {
                qWarning().noquote().nospace()
                    << "Core::ActionDomain::buildLayouts(): duplicated item id " << id;
                return false;
            }

            auto it = d->objectInfoMap.find(id);
            if (it == d->objectInfoMap.end()) {
                qWarning().noquote().nospace()
                    << "Core::ActionDomain::buildLayouts(): unknown item id " << id;
                return false;
            }
            itemMap.insert(id, {item, *it});
        }

        // Remove all menus
        for (const auto &item : items) {
            if (item->isMenu()) {
                item->d_func()->deleteAllMenus();
            }
        }
        d->sharedMenuItem->d_func()->deleteAllMenus();

        // Build layouts
        auto &fac = d->sharedMenuItem->d_func()->menuFactory;
        auto oldFac = fac;
        fac = defaultMenuFactory;
        for (const auto &item : d->layouts.value()) {
            auto pair = itemMap.value(item.id());
            if (pair.second.type() != ActionObjectInfo::Menu ||
                pair.second.shape() != ActionObjectInfo::TopLevel || !pair.first->isTopLevel()) {
                continue;
            }
            UILayoutBuildHelper(d).build(item, itemMap, nullptr);
        }
        fac = oldFac;
        return true;
    }
    void ActionDomain::updateTexts(const QList<ActionItem *> &items) const {
        Q_D(const ActionDomain);
        for (const auto &item : items) {
            auto it = d->objectInfoMap.find(item->id());
            if (it == d->objectInfoMap.end())
                continue;
            auto text = ActionObjectInfo::translatedText(it.value().text());
            switch (item->type()) {
                case ActionItem::Action: {
                    item->action()->setText(text);
                    break;
                }
                case ActionItem::Menu: {
                    for (const auto &menu : item->createdMenus()) {
                        menu->setTitle(text);
                    }
                    break;
                }
                case ActionItem::TopLevel: {
                    auto w = item->topLevel();
                    auto mo = item->topLevel()->metaObject();
                    if (auto idx = mo->indexOfProperty("text"); idx >= 0) {
                        auto prop = mo->property(idx);
                        if (prop.isWritable()) {
                            prop.write(w, text);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
        for (const auto &menu : d->sharedMenuItem->createdMenus()) {
            auto it = d->objectInfoMap.find(menu->property("action-item-id").toString());
            if (it == d->objectInfoMap.end())
                continue;
            auto text = ActionObjectInfo::translatedText(it.value().text());
            menu->setTitle(text);
        }
    }
    void ActionDomain::updateIcons(const QString &theme, const QList<ActionItem *> &items) const {
        Q_D(const ActionDomain);
        for (const auto &item : items) {
            switch (item->type()) {
                case ActionItem::Action: {
                    item->action()->setIcon(objectIcon(theme, item->id()));
                    break;
                }
                case ActionItem::Menu: {
                    for (const auto &menu : item->createdMenus()) {
                        menu->setIcon(objectIcon(theme, item->id()));
                    }
                    break;
                }
                case ActionItem::TopLevel: {
                    auto w = item->topLevel();
                    auto mo = item->topLevel()->metaObject();
                    if (auto idx = mo->indexOfProperty("icon"); idx >= 0) {
                        auto prop = mo->property(idx);
                        if (prop.isWritable()) {
                            prop.write(w, objectIcon(theme, item->id()));
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
        for (const auto &menu : d->sharedMenuItem->createdMenus()) {
            menu->setIcon(objectIcon(theme, menu->property("action-item-id").toString()));
        }
    }

    ActionDomain::ActionDomain(ActionDomainPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;

        d.init();
    }

}
