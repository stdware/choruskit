#ifndef ACTIONDOMAIN_H
#define ACTIONDOMAIN_H

#include <QObject>
#include <QJsonObject>
#include <QIcon>

#include <CoreApi/actionitem.h>
#include <CoreApi/actionextension.h>

namespace Core {

    class ActionDomain;

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

    class ActionIconMappingData;

    class CKAPPCORE_EXPORT ActionIconMapping {
    public:
        ActionIconMapping();
        ~ActionIconMapping();

        ActionIconMapping(const ActionIconMapping &other);
        ActionIconMapping(ActionIconMapping &&other) noexcept;
        ActionIconMapping &operator=(const ActionIconMapping &other);
        ActionIconMapping &operator=(ActionIconMapping &&other) noexcept;

    public:
        void addIconExtension(const QString &extensionFileName);
        void addIcon(const QString &theme, const QString &id, const QString &fileName);

        QIcon icon(const QString &theme, const QString &id) const;

    private:
        QSharedDataPointer<ActionIconMappingData> d_ptr;

        friend class ActionDomain;
    };

    class CKAPPCORE_EXPORT ActionDomain : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionDomain)
    public:
        explicit ActionDomain(QObject *parent = nullptr);
        ~ActionDomain();

    public:
        QList<const ActionExtension *> extensions() const;
        void addExtension(const ActionExtension *extension);
        void removeExtension(const ActionExtension *extension);

        QList<const ActionIconMapping *> iconMappings() const;
        void addIconMapping(const ActionIconMapping *mapping);
        void removeIconMapping(const ActionIconMapping *mapping);

    public:
        QStringList ids() const;
        ActionObjectInfo *objectInfo(const QString &id) const;

        ActionCatalogue catalogue() const;
        QList<ActionLayout> currentLayouts() const;

        QList<QKeySequence> shortcuts(const QString &id) const;
        void setShortcuts(const QString &id, const QList<QKeySequence> &shortcuts);

        QString icon(const QString &id) const;
        void setIcon(const QString &id, const QString &fileName);

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