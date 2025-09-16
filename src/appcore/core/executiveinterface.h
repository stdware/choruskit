#ifndef EXECUTIVEINTERFACE_H
#define EXECUTIVEINTERFACE_H

#include <QObject>

#include <CoreApi/objectpool.h>

namespace Core {

    class ExecutiveInterface;

    class ExecutiveInterfacePrivate;

    class ExecutiveInterfaceAddOnPrivate;

    class CKAPPCORE_EXPORT ExecutiveInterfaceAddOn : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ExecutiveInterfaceAddOn)
    public:
        explicit ExecutiveInterfaceAddOn(QObject *parent = nullptr);
        ~ExecutiveInterfaceAddOn();

        virtual void initialize() = 0;
        virtual void extensionsInitialized() = 0;
        virtual bool delayedInitialize();

    public:
        ExecutiveInterface *host() const;

    protected:
        ExecutiveInterfaceAddOn(ExecutiveInterfaceAddOnPrivate &d, QObject *parent = nullptr);

        QScopedPointer<ExecutiveInterfaceAddOnPrivate> d_ptr;

        friend class ExecutiveInterface;
        friend class ExecutiveInterfacePrivate;
    };

    class CKAPPCORE_EXPORT ExecutiveInterface : public ObjectPool {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ExecutiveInterface)
    public:
        using AddOnType = ExecutiveInterfaceAddOn;

        enum State {
            Preparatory,
            Starting,
            Initialized,
            Running,
            Exiting,
            Deleted,
        };
        Q_ENUM(State)

        ~ExecutiveInterface();

        State state() const;

        void quit();

    Q_SIGNALS:
        void initializationDone();
        void loadingStateChanged(State state);

    protected:
        void attachImpl(ExecutiveInterfaceAddOn *addOn);
        void loadImpl(bool enableDelayed = true);

    protected:
        virtual void nextLoadingState(State nextState);

    protected:
        explicit ExecutiveInterface(QObject *parent = nullptr);
        ExecutiveInterface(ExecutiveInterfacePrivate &d, QObject *parent = nullptr);
    };

    template <class HostType>
    class ExecutiveInterfaceRegistry {
    public:
        using AddOnType = typename HostType::AddOnType;
        using AddOnFactory = std::function<ExecutiveInterfaceAddOn *(QObject *)>;

    public:
        ExecutiveInterfaceRegistry() {
        }

        template <class T>
        void attach() {
            static_assert(std::is_base_of<AddOnType, T>::value, "T should inherit from ...");
            factories.append([](QObject *parent) -> ExecutiveInterfaceAddOn * {
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

#endif // EXECUTIVEINTERFACE_H
