#ifndef ACTIONDOMAIN_H
#define ACTIONDOMAIN_H

#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QToolBar>

#include <optional>

#include <QMCore/qmdisplaystring.h>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class ActionItem;

    class ActionDomain;

    using ActionDomainState = QMap<QString, QStringList>;

    class ActionDomainPrivate;

    class ActionInsertRule {
    public:
        enum InsertMode {
            Append,
            Unshift,
        };

        InsertMode mode;
        QString refItem;
        bool expandMenu;

        inline ActionInsertRule() : ActionInsertRule(Append) {
        }
        inline ActionInsertRule(InsertMode mode, bool expandMenu = false) : mode(mode), expandMenu(expandMenu) {
        }
        inline ActionInsertRule(InsertMode mode, const QString &refItem, bool expandMenu = false)
            : mode(mode), refItem(refItem), expandMenu(expandMenu) {
        }
    };

    class CKAPPCORE_EXPORT ActionDomain : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionDomain)
    public:
        enum Type {
            Action,
            Group,
            Menu,
            Separator,
            Stretch,
        };
        Q_ENUM(Type)

        explicit ActionDomain(const QString &id, QObject *parent = nullptr);
        ActionDomain(const QString &id, const QMDisplayString &title, QObject *parent = nullptr);
        ~ActionDomain();

        QString id() const;

    public:
        QString title() const;
        void setTitle(const QMDisplayString &title);

        bool configurable() const;
        void setConfigurable(bool configurable);

        bool addTopLevelMenu(const QString &id);
        bool removeTopLevelMenu(const QString &id);
        bool containsTopLevelMenu(const QString &id) const;
        QStringList topLevelMenus() const;

        bool addAction(const QString &id, Type type);
        bool removeAction(const QString &id);
        bool containsAction(const QString &id) const;
        std::optional<Type> actionType(const QString &id) const;
        QStringList actions() const;

        void addInsertRule(const QString &targetId, const QString &id, const ActionInsertRule &rule);
        void removeInsertRule(const QString &targetId, const QString &id);
        bool hasActionInsertRule(const QString &targetId, const QString &id) const;

        ActionDomainState state() const;
        ActionDomainState cachedState() const;

        void buildDomain(const QMap<QString, QWidget *> &topLevelMenus, const QList<ActionItem *> &items,
                         const ActionDomainState &state) const;

    signals:
        void stateChanged();

    protected:
        ActionDomain(ActionDomainPrivate &d, const QString &id, QObject *parent = nullptr);

        QScopedPointer<ActionDomainPrivate> d_ptr;

        friend class ActionSystem;
    };

}

#endif // ACTIONDOMAIN_H
