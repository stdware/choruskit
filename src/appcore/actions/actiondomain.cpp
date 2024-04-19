#include "actiondomain.h"
#include "actiondomain_p.h"

#include <QFileInfo>

namespace Core {

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
    QList<ActionCatalogue> ActionCatalogue::children() const {
        return d->children;
    }
    void ActionCatalogue::setChildren(const QList<ActionCatalogue> &children) {
        d->children = children;
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
        d->children = children;
    }

    ActionDomainPrivate::ActionDomainPrivate() {
    }

    ActionDomainPrivate::~ActionDomainPrivate() {
    }

    void ActionDomainPrivate::init() {
    }

    static QHash<QString, QHash<QString, QString>> parseIconConfig(const QString &fileName) {
        return {};
    }

    void ActionDomainPrivate::flushActionStructure() const {
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
                } else if (auto iconsFromFile = parseIconConfig(itemToBeChanged.fileName);
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
    ActionDomain::~ActionDomain() {
    }
    QByteArray ActionDomain::saveCurrentLayouts() const {
        return {};
    }
    bool ActionDomain::restoreCurrentLayouts(const QByteArray &obj) {
        return false;
    }
    QByteArray ActionDomain::saveOverriddenAttributes() const {
        return {};
    }
    bool ActionDomain::restoreOverriddenAttributes(const QByteArray &obj) {
        return false;
    }
    void ActionDomain::addExtension(const ActionExtension *extension) {
        Q_D(ActionDomain);
        d->extensions.append(extension);
        d->actionStructure.dirty = true;
    }
    void ActionDomain::removeExtension(const ActionExtension *extension) {
        Q_D(ActionDomain);
        d->extensions.remove(extension);
        d->actionStructure.dirty = true;
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
        return QStringList();
    }
    ActionObjectInfo ActionDomain::objectInfo(const QString &objId) const {
        return ActionObjectInfo();
    }
    ActionCatalogue ActionDomain::catalogue() const {
        Q_D(const ActionDomain);
        d->flushActionStructure();
        return d->actionStructure.catalogue;
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
    QList<ActionLayout> ActionDomain::currentLayouts() const {
        return QList<ActionLayout>();
    }
    void ActionDomain::setCurrentLayouts(const QList<ActionLayout> &layouts) {
    }
    std::optional<QList<QKeySequence>>
        ActionDomain::overriddenShortcuts(const QString &objId) const {
        Q_D(const ActionDomain);
        return d->overridedShortcuts.value(objId);
    }
    void ActionDomain::setOverriddenShortcuts(const QString &objId,
                                              const std::optional<QList<QKeySequence>> &shortcuts) {
        Q_D(ActionDomain);
        d->overridedShortcuts.insert(objId, shortcuts);
    }
    std::optional<QString> ActionDomain::overriddenIconFile(const QString &objId) const {
        Q_D(const ActionDomain);
        return d->overridedIcons.value(objId);
    }
    void ActionDomain::setOverriddenIconFile(const QString &objId,
                                             const std::optional<QString> &fileName) {
        Q_D(ActionDomain);
        if (fileName) {
            QFileInfo info(fileName.value());
            if (!info.isFile())
                return;
        }
        d->overridedIcons.insert(objId, fileName);
    }

    bool ActionDomain::buildLayouts(const QString &theme, const QList<ActionItem *> &items) const {
        return false;
    }

    ActionDomain::ActionDomain(ActionDomainPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;

        d.init();
    }

}
