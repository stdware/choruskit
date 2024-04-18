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
    QStringList ActionDomain::ids() const {
        return {};
    }
    ActionObjectInfo ActionDomain::objectInfo(const QString &id) const {
        return {};
    }
    ActionCatalogue ActionDomain::catalogue() const {
        return {};
    }
    QIcon ActionDomain::icon(const ActionLayout::IconReference &icon) const {
        return QIcon();
    }
    QIcon ActionDomain::icon(const QString &id) const {
        return QIcon();
    }
    bool ActionDomain::build(const QList<ActionItem *> &items) const {
        return false;
    }
    QList<ActionLayout> ActionDomain::currentLayouts() const {
        return {};
    }
    void ActionDomain::setCurrentLayouts(const QList<ActionLayout> &layouts) {
    }
    QList<QKeySequence> ActionDomain::shortcuts(const QString &id) const {
        return {};
    }
    void ActionDomain::setShortcuts(const QString &id, const QList<QKeySequence> &shortcuts) {
    }
    QJsonObject ActionDomain::saveLayouts() const {
        return {};
    }
    bool ActionDomain::restoreLayouts(const QJsonObject &obj) {
        return false;
    }
    QJsonObject ActionDomain::saveKeymap() const {
        return {};
    }
    bool ActionDomain::restoreKeymap(const QJsonObject &obj) {
        return false;
    }



    ActionDomain::ActionDomain(ActionDomainPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;

        d.init();
    }

}
