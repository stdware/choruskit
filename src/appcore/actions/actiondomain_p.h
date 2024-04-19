#ifndef ACTIONDOMAINPRIVATE_H
#define ACTIONDOMAINPRIVATE_H

#include <list>
#include <variant>

#include <QMCore/qmchronoset.h>
#include <QMCore/qmchronomap.h>

#include <CoreApi/actiondomain.h>

namespace Core {

    class ActionCatalogueData : public QSharedData {
    public:
        QByteArray name;
        QList<ActionCatalogue> children;
    };

    class ActionLayoutData : public QSharedData {
    public:
        QString id;
        ActionObjectInfo::Type type = ActionObjectInfo::Action;
        bool flat = false;
        ActionLayout::IconReference icon;
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
        struct ActionStructure {
            bool dirty = false;
            ActionCatalogue catalogue;
        };
        QMChronoSet<const ActionExtension *> extensions;
        mutable ActionStructure actionStructure;

        void flushActionStructure() const;

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

        QHash<QString, std::optional<QList<QKeySequence>>> overridedShortcuts;
        QHash<QString, std::optional<QString>> overridedIcons;

        void flushIcons() const;
    };

}

#endif // ACTIONDOMAINPRIVATE_H