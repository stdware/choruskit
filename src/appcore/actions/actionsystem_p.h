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

        ActionSystem *q_ptr;
    };
}

#endif // ACTIONSYSTEM_P_H
