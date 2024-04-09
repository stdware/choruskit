#ifndef WINDOWSYSTEM_H
#define WINDOWSYSTEM_H

#include <QObject>

#include <CoreApi/iwindow.h>
#include <CoreApi/iwindowaddon.h>

class QSplitter;

namespace Core {

    class WindowSystemPrivate;

    class CKAPPCORE_EXPORT WindowSystem : public QObject {
        Q_OBJECT
    public:
        explicit WindowSystem(QObject *parent = nullptr);
        ~WindowSystem();

    public:
        using AddOnFactory = std::function<IWindowAddOn *(QObject *)>;

        bool addAddOn(const QString &id, const QMetaObject *metaObject,
                      const AddOnFactory &factory = {});
        bool removeAddOn(const QString &id, const QMetaObject *metaObject);

        template <class T>
        inline void addAddOn(const QString &id);
        template <class T>
        inline void addAddOn(const QStringList &ids);
        template <class T>
        inline void removeAddOn(const QString &id);

        IWindow *findWindow(QWidget *window) const;

        int count() const;
        QList<IWindow *> windows() const;
        IWindow *firstWindow() const;

    public:
        void loadWindowGeometry(const QString &id, QWidget *w, const QSize &fallback = {}) const;
        void saveWindowGeometry(const QString &id, QWidget *w);

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

    template <class T>
    inline void WindowSystem::addAddOn(const QString &id) {
        static_assert(std::is_base_of<IWindowAddOn, T>::value,
                      "T should inherit from Core::IWindowAddOn");
        addAddOn(id, &T::staticMetaObject, [](QObject *parent) { return new T(parent); });
    }

    template <class T>
    inline void WindowSystem::addAddOn(const QStringList &ids) {
        static_assert(std::is_base_of<IWindowAddOn, T>::value,
                      "T should inherit from Core::IWindowAddOn");
        for (const auto &id : ids)
            addAddOn(id, &T::staticMetaObject, [](QObject *parent) { return new T(parent); });
    }

    template <class T>
    inline void WindowSystem::removeAddOn(const QString &id) {
        static_assert(std::is_base_of<IWindowAddOn, T>::value,
                      "T should inherit from Core::IWindowAddOn");
        removeAddOn(id, &T::staticMetaObject);
    }

}

#endif // WINDOWSYSTEM_H
