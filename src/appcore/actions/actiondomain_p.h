#ifndef ACTIONDOMAINPRIVATE_H
#define ACTIONDOMAINPRIVATE_H

#include <list>
#include <variant>

#include <QSet>

#include <QMCore/qmchronoset.h>
#include <QMCore/qmchronomap.h>

#include <CoreApi/actiondomain.h>

QT_SPECIALIZE_STD_HASH_TO_CALL_QHASH_BY_CREF(QStringList)

namespace Core {

    class ActionCatalogData : public QSharedData {
    public:
        QByteArray name;
        QString id;
        QList<ActionCatalog> children;
        QHash<QByteArray, int> indexes;
    };

    class ActionLayoutData : public QSharedData {
    public:
        QString id;
        ActionLayoutInfo::Type type = ActionLayoutInfo::Action;
        QList<ActionLayout> children;
    };

    class ActionDomainPrivate {
        Q_DECLARE_PUBLIC(ActionDomain)
    public:
        ActionDomainPrivate();
        virtual ~ActionDomainPrivate();

        void init();

        ActionDomain *q_ptr;

        // Actions
        QMChronoMap<QString, const ActionExtension *> extensions; // hash -> ext
        QMChronoMap<QString, ActionObjectInfo> objectInfoMap;     // id -> obj
        QSet<QByteArrayList> objectCategories;
        mutable std::optional<ActionCatalog> catalog;
        mutable std::optional<QList<ActionLayout>> layouts;

        void flushCatalog() const;
        void flushLayouts() const;

        // Icons
        struct IconChange {
            struct Single {
                QString theme;
                QString id;
                QString fileName;
                bool remove;
            };
            struct Config {
                QString fileName;
                bool remove;
            };
            QMChronoMap<QStringList, std::variant<Single, Config>> items;
        };
        struct IconStorage {
            QHash<QString, QHash<QString, QString>> singles;
            QHash<QString, QHash<QString, QHash<QString, QString>>> configFiles;
            QMChronoSet<QStringList> items;
            QHash<QString, QHash<QString, QString>> storage;
        };
        mutable IconChange iconChange;
        mutable IconStorage iconStorage;

        ActionDomain::ShortcutsFamily overriddenShortcuts;
        ActionDomain::IconFamily overriddenIcons;

        QScopedPointer<QWidgetAction> sharedStretchWidgetAction;
        QScopedPointer<ActionItem> sharedMenuItem;

        void flushIcons() const;

        bool setLayouts_helper(const QList<ActionLayout> &layouts) const;

        void buildLayoutsRecursively(
            const ActionLayout &layout, QWidget *parent,
            const QHash<QString, QPair<ActionItem *, ActionObjectInfo>> &itemMap,
            QHash<QWidget *, int> &lastMenuItems,
            QHash<QString, QMenu *> &autoCreatedStandaloneMenus,
            QHash<QString, ActionLayout> &standaloneLayouts) const;
    };

}

#endif // ACTIONDOMAINPRIVATE_H