#include "executiveinterface.h"
#include "executiveinterface_p.h"

namespace Core {

    static const int DELAYED_INITIALIZE_INTERVAL = 5; // ms

    ExecutiveInterfaceAddOnPrivate::ExecutiveInterfaceAddOnPrivate() {
    }

    ExecutiveInterfaceAddOnPrivate::~ExecutiveInterfaceAddOnPrivate() = default;

    void ExecutiveInterfaceAddOnPrivate::init() {
    }

    ExecutiveInterfaceAddOn::ExecutiveInterfaceAddOn(QObject *parent) : QObject(parent) {
    }

    ExecutiveInterfaceAddOn::~ExecutiveInterfaceAddOn() {
    }

    bool ExecutiveInterfaceAddOn::delayedInitialize() {
        return false;
    }

    ExecutiveInterface *ExecutiveInterfaceAddOn::host() const {
        Q_D(const ExecutiveInterfaceAddOn);
        return d->host;
    }

    ExecutiveInterfaceAddOn::ExecutiveInterfaceAddOn(ExecutiveInterfaceAddOnPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.init();
    }

    ExecutiveInterfacePrivate::ExecutiveInterfacePrivate() {
        state = ExecutiveInterface::Preparatory;
        delayedInitializeTimer = nullptr;
    }

    ExecutiveInterfacePrivate::~ExecutiveInterfacePrivate() {
        stopDelayedTimer();
    }

    void ExecutiveInterfacePrivate::init() {
    }

    void ExecutiveInterfacePrivate::load(bool enableDelayed) {
        Q_Q(ExecutiveInterface);

        // Setup
        changeLoadState(ExecutiveInterface::Starting);

        // Initialize
        for (auto &addOn : qAsConst(addOns)) {
            // Call 1
            addOn->initialize();
        }

        changeLoadState(ExecutiveInterface::Initialized);

        // ExtensionsInitialized
        for (auto it2 = addOns.rbegin(); it2 != addOns.rend(); ++it2) {
            auto &addOn = *it2;
            // Call 2
            addOn->extensionsInitialized();
        }

        // Add-ons finished
        changeLoadState(ExecutiveInterface::Running);

        if (enableDelayed) {
            // Delayed initialize
            delayedInitializeQueue = addOns;

            delayedInitializeTimer = new QTimer();
            delayedInitializeTimer->setInterval(DELAYED_INITIALIZE_INTERVAL);
            delayedInitializeTimer->setSingleShot(true);
            connect(delayedInitializeTimer, &QTimer::timeout, this,
                    &ExecutiveInterfacePrivate::nextDelayedInitialize);
            delayedInitializeTimer->start();
        } else {
            Q_EMIT q->initializationDone();
        }
    }

    void ExecutiveInterfacePrivate::quit() {
        Q_Q(ExecutiveInterface);

        stopDelayedTimer();

        changeLoadState(ExecutiveInterface::Exiting);

        // Delete addOns
        for (auto it2 = addOns.rbegin(); it2 != addOns.rend(); ++it2) {
            auto &addOn = *it2;
            addOn->deleteLater();
        }

        changeLoadState(ExecutiveInterface::Deleted);
    }

    void ExecutiveInterfacePrivate::changeLoadState(ExecutiveInterface::State newState) {
        Q_Q(ExecutiveInterface);
        q->nextLoadingState(newState);
        state = newState;
        Q_EMIT q->loadingStateChanged(newState);
    }

    void ExecutiveInterfacePrivate::stopDelayedTimer() {
        // Stop delayed initializations
        if (delayedInitializeTimer) {
            if (delayedInitializeTimer->isActive()) {
                delayedInitializeTimer->stop();
            }
            delete delayedInitializeTimer;
            delayedInitializeTimer = nullptr;
        }
    }

    void ExecutiveInterfacePrivate::nextDelayedInitialize() {
        Q_Q(ExecutiveInterface);

        while (!delayedInitializeQueue.empty()) {
            auto addOn = delayedInitializeQueue.front();
            delayedInitializeQueue.pop_front();

            bool delay = addOn->delayedInitialize();
            if (delay)
                break; // do next delayedInitialize after a delay
        }
        if (delayedInitializeQueue.empty()) {
            delete delayedInitializeTimer;
            delayedInitializeTimer = nullptr;
            Q_EMIT q->initializationDone();
        } else {
            delayedInitializeTimer->start();
        }
    }

    ExecutiveInterface::~ExecutiveInterface() {
    }

    ExecutiveInterface::State ExecutiveInterface::state() const {
        Q_D(const ExecutiveInterface);
        return d->state;
    }

    void ExecutiveInterface::quit() {
        Q_D(ExecutiveInterface);
        if (d->state != Running)
            return;
        d->quit();
    }

    void ExecutiveInterface::attachImpl(ExecutiveInterfaceAddOn *addOn) {
        Q_D(ExecutiveInterface);
        if (d->state >= Starting)
            return;
        d->addOns.append(addOn);
    }

    void ExecutiveInterface::loadImpl(bool enableDelayed) {
        Q_D(ExecutiveInterface);
        if (d->state >= Starting)
            return;

        for (auto &addOn : qAsConst(d->addOns)) {
            auto d1 = addOn->d_func();
            addOn->d_func()->host = this;
        }
        d->load(enableDelayed);
    }

    void ExecutiveInterface::nextLoadingState(State nextState) {
        Q_UNUSED(nextState);
    }

    ExecutiveInterface::ExecutiveInterface(QObject *parent) : ExecutiveInterface(*new ExecutiveInterfacePrivate(), parent) {
    }

    ExecutiveInterface::ExecutiveInterface(ExecutiveInterfacePrivate &d, QObject *parent) : ObjectPool(d, parent) {
        d.init();
    }

}
