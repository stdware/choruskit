#ifndef WINDOWINTERFACE_P_H
#define WINDOWINTERFACE_P_H

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the ChorusKit API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

#include <QHash>
#include <QSet>
#include <QTimer>
#include <QPointer>

#include <stdcorelib/linked_map.h>

#include <CoreApi/windowinterface.h>

#include <CoreApi/private/executiveinterface_p.h>

namespace Core {

    class WindowSystemPrivate;

    class WindowInterfaceAddOnPrivate : public ExecutiveInterfaceAddOnPrivate {
        Q_DECLARE_PUBLIC(WindowInterfaceAddOn)
    public:
        WindowInterfaceAddOnPrivate();
        ~WindowInterfaceAddOnPrivate();

        void init();
    };

    class CKAPPCORE_EXPORT WindowInterfacePrivate : public ExecutiveInterfacePrivate {
        Q_DECLARE_PUBLIC(WindowInterface)
    public:
        WindowInterfacePrivate();
        ~WindowInterfacePrivate();

        void init();

        void load(bool enableDelayed) override;
        void quit() override;

        bool closeAsExit;

        QObject *winFilter;
        QPointer<QWindow> window;

    private:
        friend class WindowSystem;
    };
}



#endif // WINDOWINTERFACE_P_H
