#ifndef ACTIONDOMAIN_H
#define ACTIONDOMAIN_H

#include <QObject>
#include <QJsonObject>
#include <QIcon>

#include <CoreApi/actionitem.h>
#include <CoreApi/actionextension.h>

namespace Core {

    class ActionDomain;

    class ActionCatalogueData;

    class CKAPPCORE_EXPORT ActionCatalogue {
    public:
        ActionCatalogue();
        ActionCatalogue(const QByteArray &name);
        ActionCatalogue(const ActionCatalogue &other);
        ActionCatalogue &operator=(const ActionCatalogue &other);
        ~ActionCatalogue();

    public:
        QByteArray name() const;
        void setName(const QByteArray &name);

        QList<ActionCatalogue> children() const;
        void setChildren(const QList<ActionCatalogue> &children);

    protected:
        QSharedDataPointer<ActionCatalogueData> d;

        friend class ActionDomain;
    };

    class ActionLayoutData;

    class CKAPPCORE_EXPORT ActionLayout {
    public:
        ActionLayout();
        ActionLayout(const QString &id);
        ActionLayout(const ActionLayout &other);
        ActionLayout &operator=(const ActionLayout &other);
        ~ActionLayout();

        class IconReference {
        public:
            inline IconReference(const QString &data = {}, bool fromFile = false)
                : m_fromFile(false), m_data(data) {
            }
            Q_CONSTEXPR inline bool fromFile() const {
                return m_fromFile;
            }
            inline QString data() const {
                return m_data;
            }
        private:
            bool m_fromFile;
            QString m_data;
        };

    public:
        QString id() const;
        void setId(const QString &id);

        ActionObjectInfo::Type type() const;
        void setType(ActionObjectInfo::Type type);

        bool flat() const;
        void setFlat(bool flat);

        IconReference iconReference() const;
        void setIconReference(const IconReference &icon);
        inline void setIconFile(const QString &fileName);
        inline void setIconId(const QString &id);

        QList<ActionLayout> children() const;
        void setChildren(const QList<ActionLayout> &children);

    protected:
        QSharedDataPointer<ActionLayoutData> d;

        friend class ActionDomain;
    };

    inline void ActionLayout::setIconFile(const QString &fileName) {
        setIconReference({fileName, true});
    }

    inline void ActionLayout::setIconId(const QString &id) {
        setIconReference({id, false});
    }

    class ActionIconMappingData;

    class CKAPPCORE_EXPORT ActionIconMapping {
    public:
        ActionIconMapping();
        ActionIconMapping(const ActionIconMapping &other);
        ActionIconMapping &operator=(const ActionIconMapping &other);
        ~ActionIconMapping();

    public:
        void addIconExtension(const QString &extensionFileName);
        void addIcon(const QString &theme, const QString &id, const QString &fileName);

        QIcon icon(const QString &theme, const QString &id) const;

    private:
        QSharedDataPointer<ActionIconMappingData> d;

        friend class ActionDomain;
    };

    class ActionDomainPrivate;

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
        ActionObjectInfo objectInfo(const QString &id) const;
        ActionCatalogue catalogue() const;

        QIcon icon(const ActionLayout::IconReference &icon) const;
        QIcon icon(const QString &id) const;

        bool build(const QString &theme, const QList<ActionItem *> &items) const;

        QList<ActionLayout> currentLayouts() const;
        void setCurrentLayouts(const QList<ActionLayout> &layouts);

        QList<QKeySequence> shortcuts(const QString &id) const;
        void setShortcuts(const QString &id, const QList<QKeySequence> &shortcuts);

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