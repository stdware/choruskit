#ifndef ACTIONDOMAIN_H
#define ACTIONDOMAIN_H

#include <optional>

#include <QObject>
#include <QIcon>

#include <CoreApi/actionitem.h>
#include <CoreApi/actionextension.h>

namespace Core {

    class ActionDomain;

    class ActionCatalogData;

    class CKAPPCORE_EXPORT ActionCatalog {
    public:
        ActionCatalog();
        ActionCatalog(const QByteArray &name);
        ActionCatalog(const ActionCatalog &other);
        ActionCatalog &operator=(const ActionCatalog &other);
        ~ActionCatalog();

    public:
        QByteArray name() const;
        void setName(const QByteArray &name);

        QString id() const;
        void setId(const QString &id);

        QList<ActionCatalog> children() const;
        void setChildren(const QList<ActionCatalog> &children);

        int indexOfChild(const QByteArray &name) const;

    protected:
        QSharedDataPointer<ActionCatalogData> d;

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

        ActionLayoutInfo::Type type() const;
        void setType(ActionLayoutInfo::Type type);

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

        using ShortcutsOverride = std::optional<QList<QKeySequence>>;
        using IconOverride = std::optional<IconReference>;
        using ShortcutsFamily = QHash<QString, ShortcutsOverride>;
        using IconFamily = QHash<QString, IconOverride>;

    public:
        void addExtension(const ActionExtension *extension);
        void removeExtension(const ActionExtension *extension);

        void addIcon(const QString &theme, const QString &id, const QString &fileName);
        void addIconConfiguration(const QString &fileName);
        void removeIcon(const QString &theme, const QString &id);
        void removeIconConfiguration(const QString &fileName);

    public:
        QByteArray saveLayouts() const;
        bool restoreLayouts(const QByteArray &data);

        ShortcutsFamily shortcutsFamily() const;
        void setShortcutsFamily(const ShortcutsFamily &shortcutsFamily);

        IconFamily iconFamily() const;
        void setIconFamily(const IconFamily &iconFamily);

    public:
        QStringList objectIds() const;
        ActionObjectInfo objectInfo(const QString &objId) const;
        ActionCatalog catalog() const;

        QStringList iconThemes() const;
        QStringList iconIds(const QString &theme);
        QIcon icon(const QString &theme, const QString &iconId) const;

        QList<ActionLayout> layouts() const;
        void setLayouts(const QList<ActionLayout> &layouts);
        void resetLayouts();

        ShortcutsOverride shortcuts(const QString &id) const;
        void setShortcuts(const QString &id, const ShortcutsOverride &shortcuts);
        void resetShortcuts();

        IconOverride icon(const QString &id) const;
        void setIcon(const QString &id, const IconOverride &icon);
        inline void setIconFromFile(const QString &objId, const QString &fileName);
        inline void setIconFromId(const QString &objId, const QString &iconId);
        void resetIcons();

        inline QList<QKeySequence> objectShortcuts(const QString &objId) const;
        inline QIcon objectIcon(const QString &theme, const QString &objId) const;

        bool buildLayouts(const QList<ActionItem *> &items,
                          const ActionItem::MenuFactory &defaultMenuFactory = {}) const;
        void updateTexts(const QList<ActionItem *> &items) const;
        void updateIcons(const QString &theme, const QList<ActionItem *> &items) const;

    protected:
        ActionDomain(ActionDomainPrivate &d, QObject *parent = nullptr);

        QScopedPointer<ActionDomainPrivate> d_ptr;
    };

    inline void ActionDomain::setIconFromFile(const QString &objId, const QString &fileName) {
        setIcon(objId, IconReference{fileName, true});
    }

    inline void ActionDomain::setIconFromId(const QString &objId, const QString &iconId) {
        setIcon(objId, IconReference{iconId, false});
    }

    inline QList<QKeySequence> ActionDomain::objectShortcuts(const QString &objId) const {
        if (auto o = shortcuts(objId); o) {
            return o.value();
        }
        return objectInfo(objId).shortcuts();
    }

    inline QIcon ActionDomain::objectIcon(const QString &theme, const QString &objId) const {
        if (auto o = icon(objId); o) {
            return o->fromFile() ? QIcon(o->data()) : icon(theme, o->data());
        }
        return icon(theme, objId);
    }

}

#endif // ACTIONDOMAIN_H