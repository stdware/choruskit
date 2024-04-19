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

    static bool applyIconConfig(const QString &fileName,
                                QHash<QString, QHash<QString, QString>> &out) {
        return false;
    }

    void ActionDomainPrivate::flushIcons() const {
        auto &cachedIcons = iconChange.icons;
        if (cachedIcons.empty())
            return;

        for (const auto &c : std::as_const(cachedIcons)) {
            if (c.index() == 0) {
                auto itemToBeChanged = std::get<0>(c);
                if (itemToBeChanged.remove) {
                    auto it = iconMap.find(itemToBeChanged.theme);
                    if (it != iconMap.end()) {
                        auto &map = it.value();
                        map.remove(itemToBeChanged.id);
                        if (map.isEmpty()) {
                            iconMap.erase(it);
                        }
                    }
                } else {
                    iconMap[itemToBeChanged.theme][itemToBeChanged.id] = itemToBeChanged.fileName;
                }
            } else {
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
    }
    void ActionDomain::removeExtension(const ActionExtension *extension) {
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
        auto &icons = d->iconChange.icons;
        auto &indexes = d->iconChange.indexes;
        if (!icons.empty()) {
            auto it = indexes.find({theme, id});
            if (it != indexes.end()) {
                **it = itemToBeAdded;
                return;
            }
        }
        indexes.insert({theme, id}, icons.insert(icons.end(), itemToBeAdded));
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
        auto &icons = d->iconChange.icons;
        auto &indexes = d->iconChange.indexes;
        if (!icons.empty()) {
            auto it = indexes.find({fileName});
            if (it != indexes.end()) {
                **it = itemToBeAdded;
                return;
            }
        }
        indexes.insert({fileName}, icons.insert(icons.end(), itemToBeAdded));
    }
    void ActionDomain::removeIcon(const QString &theme, const QString &id) {
        Q_D(ActionDomain);
        auto &icons = d->iconChange.icons;
        auto &indexes = d->iconChange.indexes;

        ActionDomainPrivate::IconChange::Single itemToBeRemoved{
            theme,
            id,
            {},
            true,
        };
        if (!icons.empty()) {
            auto it = indexes.find({theme, id});
            if (it != indexes.end()) {
                **it = itemToBeRemoved;
                return;
            }
        }
        indexes.insert({theme, id}, icons.insert(icons.end(), itemToBeRemoved));
    }
    void ActionDomain::removeIconConfiguration(const QString &fileName) {
        Q_D(ActionDomain);
        auto &icons = d->iconChange.icons;
        auto &indexes = d->iconChange.indexes;

        ActionDomainPrivate::IconChange::Config itemToBeRemoved{
            fileName,
            true,
        };
        if (!icons.empty()) {
            auto it = indexes.find({fileName});
            if (it != indexes.end()) {
                **it = itemToBeRemoved;
                return;
            }
        }
        indexes.insert({fileName}, icons.insert(icons.end(), itemToBeRemoved));
    }
    QStringList ActionDomain::objectIds() const {
        return QStringList();
    }
    ActionObjectInfo ActionDomain::objectInfo(const QString &objId) const {
        return ActionObjectInfo();
    }
    ActionCatalogue ActionDomain::catalogue() const {
        return ActionCatalogue();
    }
    QStringList ActionDomain::iconIds() const {
        return QStringList();
    }
    QIcon ActionDomain::icon(const QString &iconId) const {
        return QIcon();
    }
    QList<ActionLayout> ActionDomain::currentLayouts() const {
        return QList<ActionLayout>();
    }
    void ActionDomain::setCurrentLayouts(const QList<ActionLayout> &layouts) {
    }
    std::optional<QList<QKeySequence>>
        ActionDomain::overriddenShortcuts(const QString &objId) const {
        return std::optional<QList<QKeySequence>>();
    }
    void ActionDomain::setOverriddenShortcuts(const QString &objId,
                                              const std::optional<QList<QKeySequence>> &shortcuts) {
    }
    std::optional<QString> ActionDomain::overriddenIconFile(const QString &objId) const {
        return std::optional<QString>();
    }
    void ActionDomain::setOverriddenIconFile(const QString &objId,
                                             const std::optional<QString> &fileName) {
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
