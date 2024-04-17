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

    void ActionDomain::addExtension(const ActionExtension *extension) {
    }

    void ActionDomain::removeExtension(const ActionExtension *extension) {
    }

    QList<ActionExtension *> ActionDomain::extensions() const {
        return QList<ActionExtension *>();
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
