#ifndef IEXECUTIVE_H
#define IEXECUTIVE_H

#include <QObject>

#include <CoreApi/objectpool.h>

namespace Core {

    class IExecutive;

    class IExecutivePrivate;

    class IExecutiveAddOnPrivate;

    class CKAPPCORE_EXPORT IExecutiveAddOn : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(IExecutiveAddOn)
    public:
        explicit IExecutiveAddOn(QObject *parent = nullptr);
        ~IExecutiveAddOn();

        virtual void initialize() = 0;
        virtual void extensionsInitialized() = 0;
        virtual bool delayedInitialize();

    public:
        IExecutive *host() const;

    protected:
        IExecutiveAddOn(IExecutiveAddOnPrivate &d, QObject *parent = nullptr);

        QScopedPointer<IExecutiveAddOn> d_ptr;

        friend class IExecutive;
        friend class IExecutivePrivate;
    };

    class CKAPPCORE_EXPORT IExecutive : public ObjectPool {
        Q_OBJECT
        Q_DECLARE_PRIVATE(IExecutive)
    public:
        explicit IExecutive(QObject *parent = nullptr);
        ~IExecutive();

    public:
        enum State {
            Preparatory,
            Starting,
            Initialized,
            Running,
            Exiting,
            Deleted,
        };
        Q_ENUM(State)

        State state() const;

    Q_SIGNALS:
        void initializationDone();
        void loadingStateChanged(State state);

    protected:
        void attachImpl(IExecutiveAddOn *addOn);
        void loadImpl(bool enableDelayed = true);
        void quitImpl();

    protected:
        virtual void nextLoadingState(State nextState);

    protected:
        IExecutive(IExecutivePrivate &d, QObject *parent = nullptr);
    };

}

#endif // IEXECUTIVE_H