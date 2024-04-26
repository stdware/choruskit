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

#include <QMCore/qmchronomap.h>
#include <QMCore/qmchronoset.h>
#include <QMWidgets/qmshortcutcontext.h>

#include <CoreApi/iwindow.h>
#include <CoreApi/iwindowaddon.h>

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

        QString id;
        bool closeAsExit;

        QObject *winFilter;

        QMShortcutContext *shortcutCtx;
        QMChronoMap<QString, ActionItem *> actionItemMap;

        QHash<QString, QWidget *> widgetMap;

        struct DragFileHandler {
            std::function<void(const QString &)> func;
            int max;
        };
        QHash<QString, DragFileHandler> dragFileHandlerMap;

    private:
        friend class WindowSystem;
    };
}



#endif // IWINDOW_P_H
