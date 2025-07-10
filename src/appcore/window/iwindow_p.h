#ifndef IWINDOW_P_H
#define IWINDOW_P_H

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

#include <stdcorelib/linked_map.h>

#include <CoreApi/iwindow.h>

#include <CoreApi/private/iexecutive_p.h>

namespace Core {

    class WindowSystemPrivate;

    class IWindowAddOnPrivate : public IExecutiveAddOnPrivate {
        Q_DECLARE_PUBLIC(IWindowAddOn)
    public:
        IWindowAddOnPrivate();
        ~IWindowAddOnPrivate();

        void init();
    };

    class CKAPPCORE_EXPORT IWindowPrivate : public IExecutivePrivate {
        Q_DECLARE_PUBLIC(IWindow)
    public:
        IWindowPrivate();
        ~IWindowPrivate();

        void init();

        void load(bool enableDelayed) override;
        void quit() override;

        bool closeAsExit;

        QObject *winFilter;
        QWindow *window;

    private:
        friend class WindowSystem;
    };
}



#endif // IWINDOW_P_H
