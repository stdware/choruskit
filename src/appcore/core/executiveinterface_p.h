#ifndef EXECUTIVEINTERFACEPRIVATE_H
#define EXECUTIVEINTERFACEPRIVATE_H

#include <QTimer>

#include <CoreApi/executiveinterface.h>
#include <CoreApi/private/objectpool_p.h>

namespace Core {

    class CKAPPCORE_EXPORT ExecutiveInterfaceAddOnPrivate : public QObject {
        Q_DECLARE_PUBLIC(ExecutiveInterfaceAddOn)
    public:
        ExecutiveInterfaceAddOnPrivate();
        ~ExecutiveInterfaceAddOnPrivate();

        void init();

        ExecutiveInterfaceAddOn *q_ptr;

        ExecutiveInterface *host;
    };

    class CKAPPCORE_EXPORT ExecutiveInterfacePrivate : public ObjectPoolPrivate {
        Q_DECLARE_PUBLIC(ExecutiveInterface)
    public:
        ExecutiveInterfacePrivate();
        ~ExecutiveInterfacePrivate();

        void init();

        virtual void load(bool enableDelayed);
        virtual void quit();

        void changeLoadState(ExecutiveInterface::State newState);

        void stopDelayedTimer();
        void nextDelayedInitialize();

        ExecutiveInterface::State state;
        QList<ExecutiveInterfaceAddOn *> addOns;

        QTimer *delayedInitializeTimer;
        QList<ExecutiveInterfaceAddOn *> delayedInitializeQueue;
    };

}

#endif // EXECUTIVEINTERFACEPRIVATE_H
