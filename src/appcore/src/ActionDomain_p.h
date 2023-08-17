#ifndef ACTIONDOMAINPRIVATE_H
#define ACTIONDOMAINPRIVATE_H

#include "ActionDomain.h"

#include <QMChronMap.h>

namespace Core {

    class ActionDomainItemPrivate {
    public:
        explicit ActionDomainItemPrivate(int type);

        QString id;
        QMDisplayString title;
        QList<ActionInsertRuleV2> rules;
        int type;

        ActionDomain *domain;
    };

    class ActionDomainPrivate {
        Q_DECLARE_PUBLIC(ActionDomain)
    public:
        ActionDomainPrivate();
        virtual ~ActionDomainPrivate();

        void init();

        ActionDomain *q_ptr;

        QString id;
        QMDisplayString title;

        bool configurable;
        QMChronMap<QString, ActionDomainItem *> topItems;
        QMChronMap<QString, ActionDomainItem *> actions;

        mutable bool stateDirty;
        mutable bool cachedStateDirty;

        mutable ActionDomainState state;
        mutable ActionDomainState cachedState;

        void setStateDirty();
        void setCachedStateDirty();
    };

}

#endif // ACTIONDOMAINPRIVATE_H