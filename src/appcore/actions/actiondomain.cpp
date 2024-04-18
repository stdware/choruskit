#include "actiondomain.h"
#include "actiondomain_p.h"

namespace Core {

    ActionCatalogue::ActionCatalogue() {
    }
    ActionCatalogue::ActionCatalogue(const QByteArray &name) : d(new ActionCatalogueData()) {
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
    ActionLayout::ActionLayout(const QString &id) : d(new ActionLayoutData()) {
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



    ActionIconMapping::ActionIconMapping() : d(new ActionIconMappingData()) {
    }
    ActionIconMapping::ActionIconMapping(const ActionIconMapping &other) = default;
    ActionIconMapping &ActionIconMapping::operator=(const ActionIconMapping &other) = default;
    ActionIconMapping::~ActionIconMapping() = default;
    void ActionIconMapping::addIconExtension(const QString &extensionFileName) {
    }
    void ActionIconMapping::addIcon(const QString &theme, const QString &id,
                                    const QString &fileName) {
    }
    QIcon ActionIconMapping::icon(const QString &theme, const QString &id) const {
        return {};
    }



    ActionDomainPrivate::ActionDomainPrivate() {
    }
    ActionDomainPrivate::~ActionDomainPrivate() {
    }
    void ActionDomainPrivate::init() {
    }
    ActionDomain::ActionDomain(QObject *parent) : ActionDomain(*new ActionDomainPrivate(), parent) {
    }
    ActionDomain::~ActionDomain() {
    }
    QList<const ActionExtension *> ActionDomain::extensions() const {
        return {};
    }
    void ActionDomain::addExtension(const ActionExtension *extension) {
    }
    void ActionDomain::removeExtension(const ActionExtension *extension) {
    }
    QList<const ActionIconMapping *> ActionDomain::iconMappings() const {
        return {};
    }
    void ActionDomain::addIconMapping(const ActionIconMapping *mapping) {
    }
    void ActionDomain::removeIconMapping(const ActionIconMapping *mapping) {
    }
    QStringList ActionDomain::objectIds() const {
        return {};
    }
    ActionObjectInfo ActionDomain::objectInfo(const QString &objId) const {
        return {};
    }
    ActionCatalogue ActionDomain::catalogue() const {
        return {};
    }
    QStringList ActionDomain::iconIds() const {
        return {};
    }
    QIcon ActionDomain::icon(const QString &iconId) const {
        return {};
    }
    QList<ActionLayout> ActionDomain::currentLayouts() const {
        return {};
    }
    void ActionDomain::setCurrentLayouts(const QList<ActionLayout> &layouts) {
    }
    std::optional<QList<QKeySequence>>
        ActionDomain::overriddenShortcuts(const QString &objId) const {
        return {};
    }
    void ActionDomain::setOverriddenShortcuts(const QString &objId,
                                              const std::optional<QList<QKeySequence>> &shortcuts) {
    }
    std::optional<QString> ActionDomain::overriddenIconFile(const QString &objId) const {
        return {};
    }
    void ActionDomain::setOverriddenIconFile(const QString &objId,
                                             const std::optional<QString> &fileName) {
    }
    QJsonObject ActionDomain::saveCurrentLayouts() const {
        return {};
    }
    bool ActionDomain::restoreCurrentLayouts(const QJsonObject &obj) {
        return false;
    }
    QJsonObject ActionDomain::saveOverriddenAttributes() const {
        return QJsonObject();
    }
    bool ActionDomain::restoreOverriddenAttributes(const QJsonObject &obj) {
        return false;
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
