#ifndef WINDOWSYSTEM_P_H
#define WINDOWSYSTEM_P_H

#include <QHash>
#include <QSet>

#include <stdcorelib/linked_map.h>

#include <CoreApi/windowsystem.h>

namespace Core {

    struct WindowGeometry {
        QRect geometry;
        bool maximized;
        double ratio;

        WindowGeometry() : maximized(false), ratio(0){};
        WindowGeometry(const QRect &rect, bool max = false)
            : geometry(rect), maximized(max), ratio(0){};

        static WindowGeometry fromObject(const QJsonObject &obj);
        QJsonObject toObject() const;
    };

    struct SplitterSizes {
        QList<int> sizes;

        SplitterSizes(const QList<int> &sizes = {}) : sizes(sizes){};

        static SplitterSizes fromObject(const QJsonObject &obj);
        QJsonObject toObject() const;
    };

    class WindowSizeTrimmer;

    class WindowSystemPrivate : public QObject {
        Q_OBJECT
        Q_DECLARE_PUBLIC(WindowSystem)
    public:
        WindowSystemPrivate();
        virtual ~WindowSystemPrivate();

        void init();

        void readSettings();
        void saveSettings() const;

        WindowSystem *q_ptr;

        stdc::linked_map<IWindow *, int /*NOT USED*/> iWindows;
        QHash<QWidget *, IWindow *> windowMap;

        QHash<QString, WindowGeometry> winGeometries;
        QHash<QString, SplitterSizes> splitterSizes;

        void windowCreated(IWindow *iWin);
        void windowAboutToDestroy(IWindow *iWin);
    };

}

#endif // WINDOWSYSTEM_P_H
