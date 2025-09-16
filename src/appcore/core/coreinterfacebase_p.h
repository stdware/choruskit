#ifndef COREINTERFACEBASEPRIVATE_H
#define COREINTERFACEBASEPRIVATE_H

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the ChorusKit API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

#include <CoreApi/coreinterfacebase.h>

#include <QtQmlIntegration/qqmlintegration.h>

class QQmlEngine;
class QJSEngine;

namespace Core{

    class CKAPPCORE_EXPORT CoreInterfaceBasePrivate {
        Q_DECLARE_PUBLIC(CoreInterfaceBase)
    public:
        CoreInterfaceBasePrivate();
        virtual ~CoreInterfaceBasePrivate();

        void init();

        WindowSystem *windowSystem;
        DocumentSystem *documentSystem;
        SettingCatalog *settingCatalog;

        CoreInterfaceBase *q_ptr;
    };

    struct CoreInterfaceBaseForeign {
        Q_GADGET
        QML_FOREIGN(CoreInterfaceBase)
        QML_SINGLETON
        QML_NAMED_ELEMENT(CoreInterfaceBase)
    public:

        static inline CoreInterfaceBase *create(QQmlEngine *, QJSEngine *engine) {
            return CoreInterfaceBase::instance();
        }
    };

}

#endif // COREINTERFACEBASEPRIVATE_H
