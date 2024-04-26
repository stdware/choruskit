#ifndef IWINDOW_H
#define IWINDOW_H

#include <CoreApi/iexecutive.h>
#include <CoreApi/windowelementsadaptor.h>
#include <CoreApi/actionitem.h>

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
    public:
        explicit IWindowAddOn(QObject *parent = nullptr);
        ~IWindowAddOn();

        IWindow *windowHandle() const;

    protected:
        IWindowAddOn(IWindowAddOnPrivate &d, QObject *parent = nullptr);

        friend class IWindow;
        friend class IWindowPrivate;
    };

    class CKAPPCORE_EXPORT IWindow : public IExecutive, public WindowElementsAdaptor {
        Q_OBJECT
        Q_DECLARE_PRIVATE(IWindow)
    public:
        using AddOnType = IWindowAddOn;
        
        explicit IWindow(QObject *parent = nullptr);
        ~IWindow();

        inline bool isEffectivelyClosed() const;

        bool closeAsExit() const;
        void setCloseAsExit(bool on);

    public:
        void addWidget(const QString &id, QWidget *w);
        void removeWidget(const QString &id);
        QWidget *widget(const QString &id) const;
        QList<QWidget *> widgets() const;

        void addActionItem(ActionItem *item);
        inline void addActionItems(const QList<ActionItem *> &items);
        void removeActionItem(ActionItem *item);
        void removeActionItem(const QString &id);
        ActionItem *actionItem(const QString &id) const;
        QList<ActionItem *> actionItems() const;

        enum ShortcutContextPriority {
            Stable,
            Mutable,
        };
        void addShortcutContext(QWidget *w, ShortcutContextPriority priority);
        void removeShortcutContext(QWidget *w);
        QList<QWidget *> shortcutContexts() const;

        bool hasDragFileHandler(const QString &suffix);
        void setDragFileHandler(const QString &suffix,
                                const std::function<void(const QString &)> &handler,
                                int maxCount = 0);
        void removeDragFileHandler(const QString &suffix);

        template <class Func>
        void setDragFileHandler(const QString &suffix,
                                typename QtPrivate::FunctionPointer<Func>::Object *o, Func slot,
                                int maxCount = 0) {
            setDragFileHandler(suffix, std::bind(slot, o), maxCount);
        }

    Q_SIGNALS:
        void widgetAdded(const QString &id, QWidget *w);
        void aboutToRemoveWidget(const QString &id, QWidget *w);

    protected:
        virtual QWidget *createWindow(QWidget *parent) const = 0;

    protected:
        IWindow(IWindowPrivate &d, QObject *parent = nullptr);

        friend class ICore;
        friend class ICorePrivate;
        friend class Internal::CorePlugin;

    public:
        template <class T>
        inline Q_DECL_CONSTEXPR T *cast();

        template <class T>
        inline Q_DECL_CONSTEXPR const T *cast() const;
    };

    inline bool IWindow::isEffectivelyClosed() const {
        return state() >= Exiting;
    }

    inline void IWindow::addActionItems(const QList<Core::ActionItem *> &items) {
        for (const auto &item : items) {
            addActionItem(item);
        }
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
