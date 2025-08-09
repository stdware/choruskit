#ifndef ICOREBASEPRIVATE_H
#define ICOREBASEPRIVATE_H

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the ChorusKit API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

#include <CoreApi/icorebase.h>

#include <QtQmlIntegration/qqmlintegration.h>

class QQmlEngine;
class QJSEngine;

namespace Core{

    class CKAPPCORE_EXPORT ICoreBasePrivate {
        Q_DECLARE_PUBLIC(ICoreBase)
    public:
        ICoreBasePrivate();
        virtual ~ICoreBasePrivate();

        void init();

        WindowSystem *windowSystem;
        DocumentSystem *documentSystem;
        SettingCatalog *settingCatalog;

        ICoreBase *q_ptr;
    };

    struct ICoreBaseForeign {
        Q_GADGET
        QML_FOREIGN(ICoreBase)
        QML_SINGLETON
        QML_NAMED_ELEMENT(ICoreBase)
    public:

        static inline ICoreBase *create(QQmlEngine *, QJSEngine *engine) {
            return ICoreBase::instance();
        }
    };

}

#endif // ICOREBASEPRIVATE_H