#ifndef ACTIONSYSTEM_P_H
#define ACTIONSYSTEM_P_H

#include <QMCore/qmchronomap.h>
#include <QMCore/qmsimplevarexp.h>

#include <CoreApi/actionsystem.h>

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

        QMChronoMap<QString, ActionSpec *> actions;
        QMChronoMap<QString, ActionDomain *> domains;

        QHash<QString, ActionDomainState> stateCaches;
        QHash<QString, QList<QKeySequence>> shortcutsCaches;
        QHash<QString, ActionIconSpec> iconCaches;
    };
}

#endif // ACTIONSYSTEM_P_H
