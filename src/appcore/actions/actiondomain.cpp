#include "ActionDomain.h"
#include "ActionDomain_p.h"

namespace Core {

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

    ActionMetaItem *ActionDomain::staticItem(const QString &id) const {
        return nullptr;
    }

    ActionMetaLayout ActionDomain::layout() const {
        return ActionMetaLayout();
    }

    ActionMetaLayout ActionDomain::staticLayout() const {
        return ActionMetaLayout();
    }

    QList<QKeySequence> ActionDomain::shortcuts(const QString &id) const {
        return QList<QKeySequence>();
    }

    void ActionDomain::setShortcuts(const QString &id, const QList<QKeySequence> &shortcuts) {
    }

    bool ActionDomain::build(const QMap<QString, ActionItem *> &items) const {
        return false;
    }

    QJsonObject ActionDomain::saveState() const {
        return {};
    }

    bool ActionDomain::restoreState(const QJsonObject &obj) {
        return false;
    }

    ActionDomain::ActionDomain(ActionDomainPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;

        d.init();
    }

}
