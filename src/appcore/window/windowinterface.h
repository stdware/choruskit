#ifndef WINDOWINTERFACE_H
#define WINDOWINTERFACE_H

#include <functional>

#include <QWindow>

#include <CoreApi/executiveinterface.h>

namespace Core {

    namespace Internal {
        class CorePlugin;
    }

    class WindowInterface;
    class WindowInterfacePrivate;
    class WindowInterfaceAddOn;
    class WindowInterfaceAddOnPrivate;

    class CKAPPCORE_EXPORT WindowInterfaceAddOn : public ExecutiveInterfaceAddOn {
        Q_OBJECT
        Q_DECLARE_PRIVATE(WindowInterfaceAddOn)
        Q_PROPERTY(WindowInterface *windowHandle READ windowHandle CONSTANT)
    public:
        explicit WindowInterfaceAddOn(QObject *parent = nullptr);
        ~WindowInterfaceAddOn();

        WindowInterface *windowHandle() const;

    protected:
        WindowInterfaceAddOn(WindowInterfaceAddOnPrivate &d, QObject *parent = nullptr);

        friend class WindowInterface;
        friend class WindowInterfacePrivate;
    };

    class CKAPPCORE_EXPORT WindowInterface : public ExecutiveInterface {
        Q_OBJECT
        Q_DECLARE_PRIVATE(WindowInterface)
        Q_PROPERTY(QWindow *window READ window WRITE setWindow NOTIFY windowChanged)
    public:
        using AddOnType = WindowInterfaceAddOn;
        
        ~WindowInterface();

        inline bool isEffectivelyClosed() const;

        bool closeAsExit() const;
        void setCloseAsExit(bool on);

        QWindow *window() const;
        void setWindow(QWindow *w);

        void addCloseCallback(const std::function<bool()> &callback);

    protected:
        virtual QWindow *createWindow(QObject *parent) const = 0;

    protected:
        explicit WindowInterface(QObject *parent = nullptr);
        WindowInterface(WindowInterfacePrivate &d, QObject *parent = nullptr);

        friend class CoreInterface;
        friend class CoreInterfacePrivate;
        friend class Internal::CorePlugin;

    Q_SIGNALS:
        void windowChanged(QWindow *window);

    public:
        template <class T>
        inline Q_DECL_CONSTEXPR T *cast();

        template <class T>
        inline Q_DECL_CONSTEXPR const T *cast() const;
    };

    inline bool WindowInterface::isEffectivelyClosed() const {
        return state() >= Exiting;
    }

    template <class T>
    inline Q_DECL_CONSTEXPR T *WindowInterface::cast() {
        static_assert(std::is_base_of<WindowInterface, T>::value, "T should inherit from Core::WindowInterface");
        return static_cast<T *>(this);
    }

    template <class T>
    inline Q_DECL_CONSTEXPR const T *WindowInterface::cast() const {
        static_assert(std::is_base_of<WindowInterface, T>::value, "T should inherit from Core::WindowInterface");
        return static_cast<T *>(this);
    }

}

#endif // WINDOWINTERFACE_H
