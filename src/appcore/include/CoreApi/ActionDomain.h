#ifndef ACTIONDOMAIN_H
#define ACTIONDOMAIN_H

#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QToolBar>

#include <QMDisplayString.h>

#include "CkAppCoreGlobal.h"

namespace Core {

    class ActionItem;

    class ActionInsertRuleV2 {
    public:
        enum InsertMode {
            Append,
            Unshift,
            InsertBehind,
            InsertFront,
        };

        QString id;
        InsertMode direction;

        inline ActionInsertRuleV2();
        inline ActionInsertRuleV2(const QString &id);
        inline ActionInsertRuleV2(const QString &id, InsertMode direction);
    };

    inline ActionInsertRuleV2::ActionInsertRuleV2() : ActionInsertRuleV2(QString()) {
    }

    inline ActionInsertRuleV2::ActionInsertRuleV2(const QString &id) : ActionInsertRuleV2(id, Append) {
    }

    inline ActionInsertRuleV2::ActionInsertRuleV2(const QString &id, ActionInsertRuleV2::InsertMode direction)
        : id(id), direction(direction) {
    }

    class ActionDomain;

    class ActionDomainItemPrivate;

    class ActionDomainItem {
        Q_GADGET
    public:
        enum Type {
            Action,
            Group,
            Menu,
            Separator,
            Stretch,
        };
        Q_ENUM(Type)

        explicit ActionDomainItem(const QString &id, int type = Action);
        ActionDomainItem(const QString &id, const QString &title, const QList<ActionInsertRuleV2> &rules,
                         int type = Action);
        ~ActionDomainItem();

        QString id() const;
        ActionDomain *domain() const;
        int type() const;

        QString title() const;
        void setTitle(const QMDisplayString &title);

        QList<ActionInsertRuleV2> rules() const;
        void setRules(const QList<ActionInsertRuleV2> &rules);

    private:
        ActionDomainItemPrivate *d;

        Q_DISABLE_COPY_MOVE(ActionDomainItem)

        friend class ActionDomain;
    };

    using ActionDomainState = QMap<QString, QStringList>;

    class ActionDomainPrivate;

    class CKAPPCORE_API ActionDomain : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionDomain)
    public:
        explicit ActionDomain(const QString &id, QObject *parent = nullptr);
        ActionDomain(const QString &id, const QMDisplayString &title, QObject *parent = nullptr);
        ~ActionDomain();

        QString id() const;

    public:
        QString title() const;
        void setTitle(const QMDisplayString &title);

        bool configurable() const;
        void setConfigurable(bool configurable);

        bool addTopLevelItem(ActionDomainItem *item);
        bool removeTopLevelItem(const QString &id);
        bool removeTopLevelItem(ActionDomainItem *item);
        QList<ActionDomainItem *> topLevelItems() const;

        bool addAction(ActionDomainItem *item);
        bool removeAction(const QString &id);
        bool removeAction(ActionDomainItem *item);
        QList<ActionDomainItem *> actions() const;

        ActionDomainState state() const;
        ActionDomainState cachedState() const;

        void buildDomain(const QMap<QString, QWidget *> &topLevelMenus, const QList<ActionItem *> &items) const;

    signals:
        void stateChanged();

    protected:
        ActionDomain(ActionDomainPrivate &d, const QString &id, QObject *parent = nullptr);

        QScopedPointer<ActionDomainPrivate> d_ptr;

        friend class ActionSystem;
    };

}

#endif // ACTIONDOMAIN_H
