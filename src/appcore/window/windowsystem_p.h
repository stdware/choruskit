#ifndef WINDOWSYSTEM_P_H
#define WINDOWSYSTEM_P_H

#include <QHash>
#include <QSet>
#include <QTimer>
#include <QWindow>
#include <qqmlintegration.h>

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

    class WindowSizeTrimmer;

    class WindowSystemAttachedType : public QObject {
        Q_OBJECT
        QML_ANONYMOUS

        Q_PROPERTY(WindowSystem *windowSystem READ windowSystem WRITE setWindowSystem NOTIFY windowSystemChanged)
        Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)

    public:
        explicit WindowSystemAttachedType(QObject *parent = nullptr);
        ~WindowSystemAttachedType() override;

        WindowSystem *windowSystem() const;
        void setWindowSystem(WindowSystem *windowSystem);

        QString id() const;
        void setId(const QString &id);

    Q_SIGNALS:
        void windowSystemChanged();
        void idChanged();

    private Q_SLOTS:
        void onWindowGeometryChanged() const;
        void onWindowVisibilityChanged() const;
        void saveGeometryDebounced() const;

    private:
        void connectToWindow();
        void disconnectFromWindow();
        void loadGeometryIfReady();

        WindowSystem *m_windowSystem = nullptr;
        QString m_id;
        QWindow *m_window = nullptr;
        QTimer *m_saveTimer = nullptr;
    };

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

        stdc::linked_map<WindowInterface *, int /*NOT USED*/> windowInterfaces;
        QHash<QWindow *, WindowInterface *> windowMap;

        QHash<QString, WindowGeometry> winGeometries;

        bool shouldStoreGeometry{true};

        void windowCreated(WindowInterface *windowInterface);
        void windowAboutToDestroy(WindowInterface *windowInterface);
    };

}

#endif // WINDOWSYSTEM_P_H
