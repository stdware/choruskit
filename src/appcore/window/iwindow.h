#ifndef IWINDOW_H
#define IWINDOW_H

#include <CoreApi/iexecutive.h>
#include <QWindow>

namespace Core {

    namespace Internal {
        class CorePlugin;
    }

    class IWindow;
    class IWindowPrivate;
    class IWindowAddOn;
    class IWindowAddOnPrivate;

    class CKAPPCORE_EXPORT IWindowAddOn : public IExecutiveAddOn {
        Q_OBJECT
        Q_DECLARE_PRIVATE(IWindowAddOn)
        Q_PROPERTY(IWindow *windowHandle READ windowHandle CONSTANT)
    public:
        explicit IWindowAddOn(QObject *parent = nullptr);
        ~IWindowAddOn();

        IWindow *windowHandle() const;

    protected:
        IWindowAddOn(IWindowAddOnPrivate &d, QObject *parent = nullptr);

        friend class IWindow;
        friend class IWindowPrivate;
    };

    class CKAPPCORE_EXPORT IWindow : public IExecutive {
        Q_OBJECT
        Q_DECLARE_PRIVATE(IWindow)
        Q_PROPERTY(QWindow *window READ window WRITE setWindow NOTIFY windowChanged)
    public:
        using AddOnType = IWindowAddOn;
        
        ~IWindow();

        inline bool isEffectivelyClosed() const;

        bool closeAsExit() const;
        void setCloseAsExit(bool on);

        QWindow *window() const;
        void setWindow(QWindow *w);

    protected:
        virtual QWindow *createWindow(QObject *parent) const = 0;

    protected:
        explicit IWindow(QObject *parent = nullptr);
        IWindow(IWindowPrivate &d, QObject *parent = nullptr);

        friend class ICore;
        friend class ICorePrivate;
        friend class Internal::CorePlugin;

    Q_SIGNALS:
        void windowChanged(QWindow *window);

    public:
        template <class T>
        inline Q_DECL_CONSTEXPR T *cast();

        template <class T>
        inline Q_DECL_CONSTEXPR const T *cast() const;
    };

    inline bool IWindow::isEffectivelyClosed() const {
        return state() >= Exiting;
    }

    template <class T>
    inline Q_DECL_CONSTEXPR T *IWindow::cast() {
        static_assert(std::is_base_of<IWindow, T>::value, "T should inherit from Core::IWindow");
        return static_cast<T *>(this);
    }

    template <class T>
    inline Q_DECL_CONSTEXPR const T *IWindow::cast() const {
        static_assert(std::is_base_of<IWindow, T>::value, "T should inherit from Core::IWindow");
        return static_cast<T *>(this);
    }

}

#endif // IWINDOW_H
