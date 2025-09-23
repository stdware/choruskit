#ifndef CHORUSKIT_RUNTIMEINTERFACE_P_H
#define CHORUSKIT_RUNTIMEINTERFACE_P_H

#include <CoreApi/runtimeinterface.h>

#include <QtQmlIntegration/qqmlintegration.h>

namespace Core {
    struct RuntimeInterfaceForeign {
        Q_GADGET
        QML_FOREIGN(RuntimeInterface)
        QML_SINGLETON
        QML_NAMED_ELEMENT(RuntimeInterface)
    public:

        static inline RuntimeInterface *create(QQmlEngine *, QJSEngine *engine) {
            return RuntimeInterface::instance();
        }
    };
}

#endif //CHORUSKIT_RUNTIMEINTERFACE_P_H
