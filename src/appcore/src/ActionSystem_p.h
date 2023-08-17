#ifndef ACTIONSYSTEM_P_H
#define ACTIONSYSTEM_P_H

#include "ActionSystem.h"
#include "QMChronMap.h"
#include "QMSimpleVarExp.h"

#include <QMXmlAdaptor.h>

namespace Core {

    class ActionSystemPrivate {
        Q_DECLARE_PUBLIC(ActionSystem)
    public:
        ActionSystemPrivate();
        virtual ~ActionSystemPrivate();

        void init();

        void readSettings();
        void saveSettings() const;

        ActionSystem *q_ptr;

        QMSimpleVarExp configVars;

        QMChronMap<QString, ActionSpec *> actions;
        QMChronMap<QString, ActionDomain *> domains;

        QHash<QString, ActionDomainState> stateCaches;
        QHash<QString, QList<QKeySequence>> shortcutsCaches;
        QHash<QString, ActionIconSpec> iconCaches;
    };
}

#endif // ACTIONSYSTEM_P_H
