#ifndef ACTIONDOMAIN_P_H
#define ACTIONDOMAIN_P_H

#include <QSet>

#include <QMCore/qmchronomap.h>

#include <CoreApi/actiondomain.h>

namespace Core {

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
        QSet<QString> topLevelMenus;
        QHash<QString, ActionDomain::Type> actions;
        QHash<QString, QMChronoMap<QString, ActionInsertRule>> rules; // target id -> [id -> rule]

        mutable bool stateDirty;
        mutable bool cachedStateDirty;

        mutable ActionDomainState state;
        mutable ActionDomainState cachedState;

        // Use shared widget action to create stretch
        QAction *sharedWidgetAction;

        void setStateDirty();
        void setCachedStateDirty();

        ActionDomainState calcState(const ActionDomainState &existingState, const decltype(rules) &rules) const;
    };

}

#endif // ACTIONDOMAIN_P_H
