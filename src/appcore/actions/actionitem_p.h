#ifndef ACTIONITEM_P_H
#define ACTIONITEM_P_H

#include <QSet>

#include <CoreApi/actionitem.h>

namespace Core {

    class ActionItemPrivate : public QObject {
        Q_DECLARE_PUBLIC(ActionItem)
    public:
        ActionItemPrivate();
        virtual ~ActionItemPrivate();

        void init();

        ActionItem *q_ptr;

        QString id;
        ActionItem::Type type;

        QAction *action;

        QWidgetAction *sharedWidgetAction;

        ActionItem::MenuFactory menuFactory;
        QSet<QMenu *> createdMenus;

        QWidget *topLevelWidget;

        void deleteAllMenus();

    private:
        void _q_menuDestroyed(QObject *obj);
    };

}

#endif // ACTIONITEM_P_H
