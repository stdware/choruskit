#include "iexecutive.h"
#include "iexecutive_p.h"

namespace Core {

    static const int DELAYED_INITIALIZE_INTERVAL = 5; // ms

    IExecutiveAddOnPrivate::IExecutiveAddOnPrivate() {
    }

    IExecutiveAddOnPrivate::~IExecutiveAddOnPrivate() = default;

    void IExecutiveAddOnPrivate::init() {
    }

    IExecutiveAddOn::IExecutiveAddOn(QObject *parent) : QObject(parent) {
    }

    IExecutiveAddOn::~IExecutiveAddOn() {
    }

    bool IExecutiveAddOn::delayedInitialize() {
        return false;
    }

    IExecutive *IExecutiveAddOn::host() const {
        Q_D(const IExecutiveAddOn);
        return d->host;
    }

    IExecutiveAddOn::IExecutiveAddOn(IExecutiveAddOnPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.init();
    }

    IExecutivePrivate::IExecutivePrivate() {
        state = IExecutive::Preparatory;
        delayedInitializeTimer = nullptr;
    }

    IExecutivePrivate::~IExecutivePrivate() {
        stopDelayedTimer();
    }

    void IExecutivePrivate::init() {
    }

    void IExecutivePrivate::load(bool enableDelayed) {
        Q_Q(IExecutive);

        // Setup
        changeLoadState(IExecutive::Starting);

        // Initialize
        for (auto &addOn : qAsConst(addOns)) {
            // Call 1
            addOn->initialize();
        }

        changeLoadState(IExecutive::Initialized);

        // ExtensionsInitialized
        for (auto it2 = addOns.rbegin(); it2 != addOns.rend(); ++it2) {
            auto &addOn = *it2;
            // Call 2
            addOn->extensionsInitialized();
        }

        // Add-ons finished
        changeLoadState(IExecutive::Running);

        if (enableDelayed) {
            // Delayed initialize
            delayedInitializeQueue = addOns;

            delayedInitializeTimer = new QTimer();
            delayedInitializeTimer->setInterval(DELAYED_INITIALIZE_INTERVAL);
            delayedInitializeTimer->setSingleShot(true);
            connect(delayedInitializeTimer, &QTimer::timeout, this,
                    &IExecutivePrivate::nextDelayedInitialize);
            delayedInitializeTimer->start();
        } else {
            Q_EMIT q->initializationDone();
        }
    }

    void IExecutivePrivate::quit() {
        Q_Q(IExecutive);

        stopDelayedTimer();

        changeLoadState(IExecutive::Exiting);

        // Delete addOns
        for (auto it2 = addOns.rbegin(); it2 != addOns.rend(); ++it2) {
            auto &addOn = *it2;
            delete addOn;
        }

        changeLoadState(IExecutive::Deleted);
    }

    void IExecutivePrivate::changeLoadState(IExecutive::State newState) {
        Q_Q(IExecutive);
        q->nextLoadingState(newState);
        state = newState;
        Q_EMIT q->loadingStateChanged(newState);
    }

    void IExecutivePrivate::stopDelayedTimer() {
        // Stop delayed initializations
        if (delayedInitializeTimer) {
            if (delayedInitializeTimer->isActive()) {
                delayedInitializeTimer->stop();
            }
            delete delayedInitializeTimer;
            delayedInitializeTimer = nullptr;
        }
    }

    void IExecutivePrivate::nextDelayedInitialize() {
        Q_Q(IExecutive);

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

    IExecutive::IExecutive(QObject *parent) : IExecutive(*new IExecutivePrivate(), parent) {
    }

    IExecutive::~IExecutive() {
    }

    IExecutive::State IExecutive::state() const {
        Q_D(const IExecutive);
        return d->state;
    }

    void IExecutive::quit() {
        Q_D(IExecutive);
        if (d->state != Running)
            return;
        d->quit();
    }

    void IExecutive::attachImpl(IExecutiveAddOn *addOn) {
        Q_D(IExecutive);
        if (d->state >= Starting)
            return;
        d->addOns.append(addOn);
    }

    void IExecutive::loadImpl(bool enableDelayed) {
        Q_D(IExecutive);
        if (d->state >= Starting)
            return;

        for (auto &addOn : qAsConst(d->addOns)) {
            auto d1 = addOn->d_func();
            addOn->d_func()->host = this;
        }
        d->load(enableDelayed);
    }

    void IExecutive::nextLoadingState(State nextState) {
        Q_UNUSED(nextState);
    }

    IExecutive::IExecutive(IExecutivePrivate &d, QObject *parent) : ObjectPool(d, parent) {
        d.init();
    }

}
