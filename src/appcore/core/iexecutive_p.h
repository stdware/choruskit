#ifndef IEXECUTIVEPRIVATE_H
#define IEXECUTIVEPRIVATE_H

#include <QTimer>

#include <CoreApi/iexecutive.h>
#include <CoreApi/private/objectpool_p.h>

namespace Core {

    class CKAPPCORE_EXPORT IExecutiveAddOnPrivate : public QObject {
        Q_DECLARE_PUBLIC(IExecutiveAddOn)
    public:
        IExecutiveAddOnPrivate();
        ~IExecutiveAddOnPrivate();

        void init();

        IExecutiveAddOn *q_ptr;

        IExecutive *host;
    };

    class CKAPPCORE_EXPORT IExecutivePrivate : public ObjectPoolPrivate {
        Q_DECLARE_PUBLIC(IExecutive)
    public:
        IExecutivePrivate();
        ~IExecutivePrivate();

        void init();

        virtual void load(bool enableDelayed);
        virtual void quit();

        void changeLoadState(IExecutive::State newState);

        void stopDelayedTimer();
        void nextDelayedInitialize();

        IExecutive::State state;
        QList<IExecutiveAddOn *> addOns;

        QTimer *delayedInitializeTimer;
        QList<IExecutiveAddOn *> delayedInitializeQueue;
    };

}

#endif // IEXECUTIVEPRIVATE_H