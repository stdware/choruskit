#ifndef ACTIONDOMAINPRIVATE_H
#define ACTIONDOMAINPRIVATE_H

#include <list>
#include <variant>

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
            std::list<std::variant<Single, Config>> icons;
            QHash<QStringList, decltype(icons)::iterator> indexes;
        };
        struct IconStorage {
             QHash<QString, QHash<QString, QString>> map;
             QHash<QString, QHash<QString, QHash<QString, QString>>> configFiles;

        };
        mutable IconChange iconChange;
        mutable IconStorage iconStorage;

        void flushIcons() const;
    };

}

#endif // ACTIONDOMAINPRIVATE_H