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

    class CKAPPCORE_EXPORT ActionCatalogue {
    public:
        inline ActionCatalogue() : data(nullptr), idx(0){};

        QByteArray name() const;

        int childCount() const;
        ActionCatalogue child(int index) const;

    private:
        const void *data;
        int idx;

        friend class ActionDomain;
    };

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
        ActionObjectInfo *objectInfo(const QString &id) const;

        ActionCatalogue catalogue() const;
        QList<ActionLayout> currentLayouts() const;

        QList<QKeySequence> shortcuts(const QString &id) const;
        void setShortcuts(const QString &id, const QList<QKeySequence> &shortcuts);

        bool build(const QMap<QString, ActionItem *> &items) const;

        QJsonObject saveLayouts() const;
        bool restoreLayouts(const QJsonObject &obj);

        QJsonObject saveKeymap() const;
        bool restoreKeymap(const QJsonObject &obj);

    protected:
        ActionDomain(ActionDomainPrivate &d, QObject *parent = nullptr);

        QScopedPointer<ActionDomainPrivate> d_ptr;
    };

}

#endif // ACTIONDOMAIN_H