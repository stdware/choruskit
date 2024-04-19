#ifndef ACTIONDOMAINPRIVATE_H
#define ACTIONDOMAINPRIVATE_H

#include <list>
#include <variant>

#include <QMCore/qmchronoset.h>
#include <QMCore/qmchronomap.h>

#include <CoreApi/actiondomain.h>

#include <CoreApi/private/actionextension_p.h>

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
        ActionObjectInfo::Type type = ActionObjectInfo::Action;
        bool flat = false;
        ActionLayout::IconReference icon;
        QList<ActionLayout> children;
        QHash<QString, int> indexes;
    };

    class ActionDomainPrivate {
        Q_DECLARE_PUBLIC(ActionDomain)
    public:
        ActionDomainPrivate();
        virtual ~ActionDomainPrivate();

        void init();

        ActionDomain *q_ptr;

        // Actions
        QMChronoMap<QString, const ActionExtension *> extensions;         // hash -> ext
        QMChronoMap<QString, const ActionObjectInfoData *> objectInfoMap; // id -> obj
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
        QHash<QString, std::optional<QString>> overriddenIcons;

        void flushIcons() const;
    };

}

#endif // ACTIONDOMAINPRIVATE_H