#ifndef ACTIONDOMAINPRIVATE_H
#define ACTIONDOMAINPRIVATE_H

#include <list>
#include <variant>

#include <QSet>

#include <QMCore/qmchronoset.h>
#include <QMCore/qmchronomap.h>

#include <CoreApi/actiondomain.h>

namespace Core {

    class ActionCatalogueData : public QSharedData {
    public:
        QByteArray name;
        QString id;
        QList<ActionCatalogue> children;
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
        mutable std::optional<ActionCatalogue> catalogue;
        mutable std::optional<QList<ActionLayout>> layouts;

        void flushCatalogue() const;
        void flushLayouts() const;

        struct StringListHash {
            size_t operator()(const QStringList &s) const noexcept {
                return qHash(s);
            }
        };

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
            QMChronoMap<QStringList, std::variant<Single, Config>, StringListHash> items;
        };
        struct IconStorage {
            QHash<QString, QHash<QString, QString>> singles;
            QHash<QString, QHash<QString, QHash<QString, QString>>> configFiles;
            QMChronoSet<QStringList, StringListHash> items;
            QHash<QString, QHash<QString, QString>> storage;
        };
        mutable IconChange iconChange;
        mutable IconStorage iconStorage;

        QHash<QString, std::optional<QList<QKeySequence>>> overriddenShortcuts;
        QHash<QString, std::optional<ActionDomain::IconReference>> overriddenIcons;

        QScopedPointer<QWidgetAction> sharedStretchWidgetAction;
        QScopedPointer<ActionItem> sharedMenuItem;

        void flushIcons() const;
    };

}

#endif // ACTIONDOMAINPRIVATE_H