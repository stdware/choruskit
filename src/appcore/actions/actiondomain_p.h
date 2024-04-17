#ifndef ACTIONDOMAINPRIVATE_H
#define ACTIONDOMAINPRIVATE_H

#include <CoreApi/actiondomain.h>

namespace Core {

    struct ActionCatalogueData {
        struct Entry {
            QByteArray name;
            QVector<int> childIndexes;
        };
        QVector<Entry> entryData;
    };

    class ActionIconMappingData : public QSharedData {
    public:
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