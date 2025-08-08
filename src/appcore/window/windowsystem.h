#ifndef WINDOWSYSTEM_H
#define WINDOWSYSTEM_H

#include <QObject>
#include <QWindow>

#include <CoreApi/iwindow.h>

namespace Core {

    class WindowSystemPrivate;

    class CKAPPCORE_EXPORT WindowSystem : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(WindowSystem)
    public:
        explicit WindowSystem(QObject *parent = nullptr);
        ~WindowSystem();

    public:
        IWindow *findWindow(QWindow *window) const;
        int count() const;
        QList<IWindow *> windows() const;
        IWindow *firstWindow() const;
        template <class T>
        T *firstWindowOfType() const {
            return qobject_cast<T *>(firstWindowOfTypeImpl(T::staticMetaObject));
        }

    public:
        void loadGeometry(const QString &id, QWindow *w, const QSize &fallback = {}) const;
        void saveGeometry(const QString &id, QWindow *w);

    Q_SIGNALS:
        void windowCreated(IWindow *iWin);
        void windowAboutToDestroy(IWindow *iWin);

    protected:
        WindowSystem(WindowSystemPrivate &d, QObject *parent = nullptr);

        QScopedPointer<WindowSystemPrivate> d_ptr;

        friend class IWindow;
        friend class IWindowPrivate;

    private:
        IWindow *firstWindowOfTypeImpl(const QMetaObject &type) const;
    };

}

#endif // WINDOWSYSTEM_H
