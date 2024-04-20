#ifndef ACTIONDOMAIN_H
#define ACTIONDOMAIN_H

#include <optional>

#include <QObject>
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

        QString id() const;
        void setId(const QString &id);

        QList<ActionCatalogue> children() const;
        void setChildren(const QList<ActionCatalogue> &children);

        int indexOfChild(const QByteArray &name) const;

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

    public:
        QString id() const;
        void setId(const QString &id);

        ActionObjectInfo::Type type() const;
        void setType(ActionObjectInfo::Type type);

        bool flat() const;
        void setFlat(bool flat);

        QList<ActionLayout> children() const;
        void addChild(const ActionLayout &child);
        void setChildren(const QList<ActionLayout> &children);

    protected:
        QSharedDataPointer<ActionLayoutData> d;

        friend class ActionDomain;
    };

    class ActionDomainPrivate;

    class CKAPPCORE_EXPORT ActionDomain : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionDomain)
    public:
        explicit ActionDomain(QObject *parent = nullptr);
        ~ActionDomain();

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
        void addExtension(const ActionExtension *extension);
        void removeExtension(const ActionExtension *extension);

        void addIcon(const QString &theme, const QString &id, const QString &fileName);
        void addIconConfiguration(const QString &fileName);
        void removeIcon(const QString &theme, const QString &id);
        void removeIconConfiguration(const QString &fileName);

    public:
        QByteArray saveLayouts() const;
        bool restoreLayouts(const QByteArray &obj);

        QByteArray saveOverriddenShortcuts() const;
        bool restoreOverriddenShortcuts(const QByteArray &obj);

        QByteArray saveOverriddenIcons() const;
        bool restoreOverriddenIcons(const QByteArray &obj);

    public:
        QStringList objectIds() const;
        ActionObjectInfo objectInfo(const QString &objId) const;
        ActionCatalogue catalogue() const;

        QStringList iconThemes() const;
        QStringList iconIds(const QString &theme);
        QIcon icon(const QString &theme, const QString &iconId) const;

        QList<ActionLayout> layouts() const;
        void setLayouts(const QList<ActionLayout> &layouts);
        void resetLayouts();

        std::optional<QList<QKeySequence>> overriddenShortcuts(const QString &objId) const;
        void setOverriddenShortcuts(const QString &objId,
                                    const std::optional<QList<QKeySequence>> &shortcuts);
        void resetShortcuts();

        std::optional<IconReference> overriddenIcon(const QString &objId) const;
        void setOverriddenIcon(const QString &objId, const std::optional<IconReference> &iconRef);
        inline void setOverriddenIconFile(const QString &objId, const QString &fileName);
        inline void setOverriddenIconId(const QString &objId, const QString &iconId);
        void resetIconFiles();

        inline QIcon objectIcon(const QString &theme, const QString &objId) const;
        inline QList<QKeySequence> objectShortcuts(const QString &objId) const;

        bool buildLayouts(const QString &theme, const QList<ActionItem *> &items) const;

    protected:
        ActionDomain(ActionDomainPrivate &d, QObject *parent = nullptr);

        QScopedPointer<ActionDomainPrivate> d_ptr;
    };

    inline void ActionDomain::setOverriddenIconFile(const QString &objId, const QString &fileName) {
        setOverriddenIcon(objId, IconReference{fileName, true});
    }

    inline void ActionDomain::setOverriddenIconId(const QString &objId, const QString &iconId) {
        setOverriddenIcon(objId, IconReference{iconId, false});
    }

    inline QIcon ActionDomain::objectIcon(const QString &theme, const QString &objId) const {
        if (auto o = overriddenIcon(objId); o) {
            return o->fromFile() ? QIcon(o->data()) : icon(theme, o->data());
        }
        return icon(theme, objId);
    }

    inline QList<QKeySequence> ActionDomain::objectShortcuts(const QString &objId) const {
        if (auto o = overriddenShortcuts(objId); o) {
            return o.value();
        }
        return objectInfo(objId).shortcuts();
    }

}

#endif // ACTIONDOMAIN_H