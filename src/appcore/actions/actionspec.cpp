#include "actionspec.h"
#include "actionspec_p.h"

#include "icorebase.h"

namespace Core {

    ActionIconSpec::ActionIconSpec(ActionSpec *spec) : m_file(false) {
        if (!spec)
            return;
        m_data = spec->id();
    }

    QIcon ActionIconSpec::icon() const {
        if (m_file)
            return QIcon(m_data);

        auto system = ICoreBase::instance()->actionSystem();
        auto spec = system->action(m_data);
        if (!spec)
            return {};
        return spec->icon();
    }

    ActionSpecPrivate::ActionSpecPrivate() {
    }

    ActionSpecPrivate::~ActionSpecPrivate() {
    }

    void ActionSpecPrivate::init() {
    }

    ActionSpec::ActionSpec(const QString &id, QObject *parent) : ActionSpec(*new ActionSpecPrivate(), id, parent) {
    }

    ActionSpec::~ActionSpec() {
    }

    QString ActionSpec::id() const {
        Q_D(const ActionSpec);
        return d->id;
    }

    QString ActionSpec::commandName() const {
        Q_D(const ActionSpec);
        return d->commandName;
    }

    void ActionSpec::setCommandName(const QString &name) {
        Q_D(ActionSpec);
        d->commandName = name;
    }

    QString ActionSpec::commandDisplayName() const {
        Q_D(const ActionSpec);
        return d->commandDisplayName;
    }

    void ActionSpec::setCommandDisplayName(const QString &displayName) {
        Q_D(ActionSpec);
        d->commandDisplayName = displayName;
    }

    QList<QKeySequence> ActionSpec::shortcuts() const {
        Q_D(const ActionSpec);
        return d->shortcuts;
    }

    void ActionSpec::setShortcuts(const QList<QKeySequence> &shortcuts) {
        Q_D(ActionSpec);
        d->shortcuts = shortcuts;
        emit shortcutsChanged();
    }

    QList<QKeySequence> ActionSpec::cachedShortcuts() const {
        auto system = qobject_cast<ActionSystem *>(parent());
        return system ? system->shortcutsCache(id()) : shortcuts();
    }

    QIcon ActionSpec::icon() const {
        Q_D(const ActionSpec);
        return d->icon;
    }

    void ActionSpec::setIcon(const QIcon &icon) {
        Q_D(ActionSpec);
        d->icon = icon;
        emit iconChanged();
    }

    QIcon ActionSpec::cachedIcon() const {
        auto system = qobject_cast<ActionSystem *>(parent());
        auto icon = system ? system->iconCache(id()).icon() : QIcon();
        return icon.isNull() ? this->icon() : icon;
    }

    ActionSpec::ActionSpec(ActionSpecPrivate &d, const QString &id, QObject *parent) : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.id = id;

        d.init();
    }

}
