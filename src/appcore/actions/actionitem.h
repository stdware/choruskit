#ifndef ACTIONITEM_H
#define ACTIONITEM_H

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QWidget>
#include <QWidgetAction>

#include <CoreApi/actionspec.h>

namespace Core {

    class ActionItemPrivate;

    class CKAPPCORE_EXPORT ActionItem : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionItem)
    public:
        enum Type {
            Invalid = 0,
            Action = 1,
            Menu = 2,
            Widget = 4,
        };

        ActionItem(const QString &id, QAction *action, QObject *parent = nullptr);
        ActionItem(const QString &id, const std::function<QWidget *(QWidget *)> &factory, QObject *parent = nullptr);
        ActionItem(const QString &id, QMenu *menu, QObject *parent = nullptr);
        ~ActionItem();

        QString id() const;
        Type type() const;
        ActionSpec *spec() const;

        inline bool isAction() const;
        inline bool isMenu() const;
        inline bool isWidget() const;

        QAction *action() const;
        QMenu *menu() const;
        QWidgetAction *widgetAction() const;
        QList<QWidget *> widgets() const;

        QIcon icon() const;
        void setIcon(const QIcon &icon);

        QString text() const;
        void setText(const QString &text);

        bool enabled() const;
        void setEnabled(bool enabled);

        bool autoDelete() const;
        void setAutoDelete(bool autoDelete);

    public:
        QString commandName() const;
        QString commandDisplayName() const;

        QString commandSpecificName() const;
        void setCommandSpecificName(const QString &commandSpecificName);

        QPair<QString, QString> commandCheckedDescription() const;
        void setCommandCheckedDescription(const QPair<QString, QString> &commandCheckedDescription);

    public:
        static bool autoDeleteGlobal();
        static void setAutoDeleteGlobal(bool autoDelete);

    protected:
        ActionItem(ActionItemPrivate &d, const QString &id, QObject *parent = nullptr);

        QScopedPointer<ActionItemPrivate> d_ptr;
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

}

#endif // ACTIONITEM_H
