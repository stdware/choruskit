#include "actiondomain.h"
#include "actiondomain_p.h"

namespace Core {

    QByteArray ActionCatalogue::name() const {
        return static_cast<const ActionCatalogueData *>(data)->entryData[idx].name;
    }

    int ActionCatalogue::childCount() const {
        return static_cast<const ActionCatalogueData *>(data)->entryData[idx].childIndexes.size();
    }

    ActionCatalogue ActionCatalogue::child(int index) const {
        ActionCatalogue result = *this;
        result.idx =
            static_cast<const ActionCatalogueData *>(data)->entryData[idx].childIndexes[index];
        return result;
    }

    ActionIconMapping::ActionIconMapping() {
    }

    ActionIconMapping::~ActionIconMapping() {
    }

    ActionIconMapping::ActionIconMapping(const ActionIconMapping &other) {
    }

    ActionIconMapping::ActionIconMapping(ActionIconMapping &&other) noexcept {
    }

    ActionIconMapping &ActionIconMapping::operator=(const ActionIconMapping &other) {
        return *this;
    }

    ActionIconMapping &ActionIconMapping::operator=(ActionIconMapping &&other) noexcept {
        return *this;
    }

    void ActionIconMapping::addIconExtension(const QString &extensionFileName) {
    }

    void ActionIconMapping::addIcon(const QString &theme, const QString &id,
                                    const QString &fileName) {
    }

    QIcon ActionIconMapping::icon(const QString &theme, const QString &id) const {
        return QIcon();
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
        return QList<const ActionExtension *>();
    }

    void ActionDomain::addExtension(const ActionExtension *extension) {
    }

    void ActionDomain::removeExtension(const ActionExtension *extension) {
    }

    QList<const ActionIconMapping *> ActionDomain::iconMappings() const {
        return QList<const ActionIconMapping *>();
    }

    void ActionDomain::addIconMapping(const ActionIconMapping *mapping) {
    }

    void ActionDomain::removeIconMapping(const ActionIconMapping *mapping) {
    }

    QStringList ActionDomain::ids() const {
        return QStringList();
    }

    ActionObjectInfo *ActionDomain::objectInfo(const QString &id) const {
        return nullptr;
    }

    ActionCatalogue ActionDomain::catalogue() const {
        return ActionCatalogue();
    }

    QList<ActionLayout> ActionDomain::currentLayouts() const {
        return QList<ActionLayout>();
    }

    QList<QKeySequence> ActionDomain::shortcuts(const QString &id) const {
        return {};
    }

    void ActionDomain::setShortcuts(const QString &id, const QList<QKeySequence> &shortcuts) {
    }

    QString ActionDomain::icon(const QString &id) const {
        return QString();
    }

    void ActionDomain::setIcon(const QString &id, const QString &fileName) {
    }

    bool ActionDomain::build(const QMap<QString, ActionItem *> &items) const {
        return false;
    }

    QJsonObject ActionDomain::saveLayouts() const {
        return QJsonObject();
    }

    bool ActionDomain::restoreLayouts(const QJsonObject &obj) {
        return false;
    }

    QJsonObject ActionDomain::saveKeymap() const {
        return QJsonObject();
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
