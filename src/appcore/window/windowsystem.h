#ifndef WINDOWSYSTEM_H
#define WINDOWSYSTEM_H

#include <QObject>
#include <QWindow>
#include <qqmlintegration.h>

#include <CoreApi/windowinterface.h>

namespace Core {

    class WindowSystemPrivate;
    class WindowSystemAttachedType;

    class CKAPPCORE_EXPORT WindowSystem : public QObject {
        Q_OBJECT
        QML_ELEMENT
        QML_ATTACHED(WindowSystemAttachedType)
        QML_UNCREATABLE("")
        Q_DECLARE_PRIVATE(WindowSystem)
        Q_PROPERTY(bool shouldStoreGeometry READ shouldStoreGeometry WRITE setShouldStoreGeometry NOTIFY shouldStoreGeometryChanged)
    public:
        explicit WindowSystem(QObject *parent = nullptr);
        ~WindowSystem() override;

        static WindowSystemAttachedType *qmlAttachedProperties(QObject *object);

    public:
        WindowInterface *findWindow(QWindow *window) const;
        int count() const;
        QList<WindowInterface *> windows() const;
        WindowInterface *firstWindow() const;
        template <class T>
        T *firstWindowOfType() const {
            return qobject_cast<T *>(firstWindowOfTypeImpl(T::staticMetaObject));
        }

    public:
        Q_INVOKABLE void loadGeometry(const QString &id, QWindow *w, const QSize &fallback = {}) const;
        Q_INVOKABLE void saveGeometry(const QString &id, QWindow *w);

        bool shouldStoreGeometry() const;
        void setShouldStoreGeometry(bool on);

    Q_SIGNALS:
        void windowCreated(WindowInterface *windowInterface);
        void windowAboutToDestroy(WindowInterface *windowInterface);
        void shouldStoreGeometryChanged(bool on);

    protected:
        WindowSystem(WindowSystemPrivate &d, QObject *parent = nullptr);

        QScopedPointer<WindowSystemPrivate> d_ptr;

        friend class WindowInterface;
        friend class WindowInterfacePrivate;

    private:
        WindowInterface *firstWindowOfTypeImpl(const QMetaObject &type) const;
    };

}

#endif // WINDOWSYSTEM_H
