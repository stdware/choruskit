#ifndef ACTIONDOMAIN_H
#define ACTIONDOMAIN_H

#include <QObject>
#include <QJsonObject>

#include <CoreApi/actionextension.h>

namespace Core {

    class ActionItem;

    class ActionDomain;

    class ActionLayoutPrivate;

    class ActionDomainPrivate;

    class CKAPPCORE_EXPORT ActionDomain : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionDomain)
    public:
        explicit ActionDomain(QObject *parent = nullptr);
        ~ActionDomain();

    public:
        void addExtension(const ActionExtension *extension);
        void removeExtension(const ActionExtension *extension);

        QList<ActionExtension *> extensions() const;

    public:
        QStringList ids() const;
        ActionMetaItem *staticItem(const QString &id) const;

        ActionMetaLayout layout() const;
        ActionMetaLayout staticLayout() const;

        QList<QKeySequence> shortcuts(const QString &id) const;
        void setShortcuts(const QString &id, const QList<QKeySequence> &shortcuts);

        bool build(const QMap<QString, ActionItem *> &items) const;

        QJsonObject saveState() const;
        bool restoreState(const QJsonObject &obj);

    protected:
        ActionDomain(ActionDomainPrivate &d, QObject *parent = nullptr);

        QScopedPointer<ActionDomainPrivate> d_ptr;
    };

}

#endif // ACTIONDOMAIN_H