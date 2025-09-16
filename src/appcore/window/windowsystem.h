#ifndef WINDOWSYSTEM_H
#define WINDOWSYSTEM_H

#include <QObject>
#include <QWindow>

#include <CoreApi/windowinterface.h>

namespace Core {

    class WindowSystemPrivate;

    class CKAPPCORE_EXPORT WindowSystem : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(WindowSystem)
    public:
        explicit WindowSystem(QObject *parent = nullptr);
        ~WindowSystem();

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
        void loadGeometry(const QString &id, QWindow *w, const QSize &fallback = {}) const;
        void saveGeometry(const QString &id, QWindow *w);

    Q_SIGNALS:
        void windowCreated(WindowInterface *windowInterface);
        void windowAboutToDestroy(WindowInterface *windowInterface);

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
