#ifndef ACTIONDOMAINPRIVATE_H
#define ACTIONDOMAINPRIVATE_H

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
    };

}

#endif // ACTIONDOMAINPRIVATE_H