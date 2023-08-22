#ifndef ACTIONSYSTEM_H
#define ACTIONSYSTEM_H

#include <QMenuBar>

#include "ActionDomain.h"
#include "ActionSpec.h"

namespace Core {

    class ActionSystemPrivate;

    class CKAPPCORE_API ActionSystem : public QObject {
        Q_OBJECT
    public:
        explicit ActionSystem(QObject *parent = nullptr);
        ~ActionSystem();

    public:
        bool addAction(ActionSpec *action);
        bool removeAction(ActionSpec *action);
        bool removeAction(const QString &id);
        ActionSpec *action(const QString &id) const;
        QList<ActionSpec *> actions() const;
        QStringList actionIds() const;

        bool addDomain(ActionDomain *domain);
        bool removeDomain(ActionDomain *domain);
        bool removeDomain(const QString &id);
        ActionDomain *domain(const QString &id) const;
        QList<ActionDomain *> domains() const;
        QStringList domainIds() const;

        bool loadDomains(const QString &fileName); // Load actions and domains from XML file

    public:
        ActionDomainState stateCache(const QString &domainId);
        void setStateCache(const QString &domainId, const ActionDomainState &state);

        QList<QKeySequence> shortcutsCache(const QString &actionId);
        void setShortcutsCache(const QString &actionId, const QList<QKeySequence> &shortcutsCache);

        ActionIconSpec iconCache(const QString &actionId);
        void setIconCache(const QString &actionId, const ActionIconSpec &iconCache);

    protected:
        QScopedPointer<ActionSystemPrivate> d_ptr;
        ActionSystem(ActionSystemPrivate &d, QObject *parent = nullptr);

        Q_DECLARE_PRIVATE(ActionSystem)

        friend class ActionContext;
        friend class ActionItem;
    };

}

#endif // ACTIONSYSTEM_H
