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

        QScopedPointer<IExecutiveAddOnPrivate> d_ptr;

        friend class IExecutive;
        friend class IExecutivePrivate;
    };

    class CKAPPCORE_EXPORT IExecutive : public ObjectPool {
        Q_OBJECT
        Q_DECLARE_PRIVATE(IExecutive)
    public:
        using AddOnType = IExecutiveAddOn;

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

        void quit();

    Q_SIGNALS:
        void initializationDone();
        void loadingStateChanged(State state);

    protected:
        void attachImpl(IExecutiveAddOn *addOn);
        void loadImpl(bool enableDelayed = true);

    protected:
        virtual void nextLoadingState(State nextState);

    protected:
        IExecutive(IExecutivePrivate &d, QObject *parent = nullptr);

        template <class T1>
        friend class IExecutiveRegistry;
    };

    template <class HostType>
    class IExecutiveRegistry {
    public:
        using AddOnType = typename HostType::AddOnType;
        using AddOnFactory = std::function<IExecutiveAddOn *(QObject *)>;

    public:
        IExecutiveRegistry() {
        }

        template <class T>
        void attach() {
            static_assert(std::is_base_of<AddOnType, T>::value, "T should inherit from ...");
            factories.append([](QObject *parent) -> IExecutiveAddOn * {
                return new T(parent); //
            });
        }

        template <class... Args>
        HostType *create(Args &&...args) const {
            auto e = new HostType(args...);
            for (const auto &fac : factories) {
                e->attachImpl(fac(e));
            }
            e->loadImpl();
            return e;
        }

    protected:
        QList<AddOnFactory> factories;
    };

}

#endif // IEXECUTIVE_H