#ifndef WINDOWSYSTEM_H
#define WINDOWSYSTEM_H

#include <QObject>

#include <CoreApi/iwindow.h>

class QSplitter;

namespace Core {

    class WindowSystemPrivate;

    class CKAPPCORE_EXPORT WindowSystem : public QObject {
        Q_OBJECT
    public:
        explicit WindowSystem(QObject *parent = nullptr);
        ~WindowSystem();

    public:
        IWindow *findWindow(QWidget *window) const;
        int count() const;
        QList<IWindow *> windows() const;
        IWindow *firstWindow() const;

    public:
        void loadGeometry(const QString &id, QWidget *w, const QSize &fallback = {}) const;
        void saveGeometry(const QString &id, QWidget *w);

        void loadSplitterSizes(const QString &id, QSplitter *s,
                               const QList<int> &fallback = {}) const;
        void saveSplitterSizes(const QString &id, QSplitter *s);

    Q_SIGNALS:
        void windowCreated(IWindow *iWin);
        void windowAboutToDestroy(IWindow *iWin);

    protected:
        QScopedPointer<WindowSystemPrivate> d_ptr;
        WindowSystem(WindowSystemPrivate &d, QObject *parent = nullptr);

        Q_DECLARE_PRIVATE(WindowSystem)

        friend class IWindow;
        friend class IWindowPrivate;
    };

}

#endif // WINDOWSYSTEM_H
