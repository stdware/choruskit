#ifndef ACTIONSYSTEM_H
#define ACTIONSYSTEM_H

#include <QMenuBar>

#include <CoreApi/actiondomain.h>

namespace Core {

    class ActionSystemPrivate;

    class CKAPPCORE_EXPORT ActionSystem : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionSystem)
    public:
        explicit ActionSystem(QObject *parent = nullptr);
        ~ActionSystem();

    public:
        // TODO

    protected:
        QScopedPointer<ActionSystemPrivate> d_ptr;
        ActionSystem(ActionSystemPrivate &d, QObject *parent = nullptr);
    };

}

#endif // ACTIONSYSTEM_H
