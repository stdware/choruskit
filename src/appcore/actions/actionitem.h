#ifndef ACTIONITEM_H
#define ACTIONITEM_H

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QWidget>
#include <QWidgetAction>
#include <QMenuBar>
#include <QToolBar>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class ActionDomain;

    class ActionItemPrivate;

    class CKAPPCORE_EXPORT ActionItem : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionItem)
    public:
        enum Type {
            Action = 1,
            Menu = 2,
            Widget = 4,
            Standalone = 8,
        };

        using MenuFactory = std::function<QMenu *(QWidget *)>;
        using WidgetFactory = std::function<QWidget *(QWidget *)>;

        ActionItem(const QString &id, QAction *action, QObject *parent = nullptr);
        ActionItem(const QString &id, const WidgetFactory &fac, QObject *parent = nullptr);
        ActionItem(const QString &id, const MenuFactory &fac, QObject *parent = nullptr);
        ActionItem(const QString &id, QWidget *topLevelWidget, QObject *parent = nullptr);
        ~ActionItem();

        QString id() const;
        Type type() const;

        inline bool isAction() const;
        inline bool isMenu() const;
        inline bool isWidget() const;
        inline bool isStandalone() const;

        QAction *action() const;
        QWidgetAction *widgetAction() const;
        QWidget *standalone() const;
        QList<QWidget *> createdWidgets() const;
        QList<QMenu *> createdMenus() const;

        QMenu *requestMenu(QWidget *parent);
        void addMenuAsRequested(QMenu *menu);

    protected:
        ActionItem(ActionItemPrivate &d, const QString &id, QObject *parent = nullptr);

        QScopedPointer<ActionItemPrivate> d_ptr;

        friend class ActionDomain;
    };

    inline bool ActionItem::isAction() const {
        return type() == Action;
    }

    inline bool ActionItem::isMenu() const {
        return type() == Menu;
    }

    inline bool ActionItem::isWidget() const {
        return type() == Widget;
    }

    inline bool ActionItem::isStandalone() const {
        return type() == Standalone;
    }

}

#endif // ACTIONITEM_H
