#ifndef OBJECTPOOL_P_H
#define OBJECTPOOL_P_H

#include <stdcorelib/linked_map.h>

#include <CoreApi/objectpool.h>

namespace Core {

    class ObjectPoolPrivate : public QObject {
        Q_DECLARE_PUBLIC(ObjectPool)
    public:
        ObjectPoolPrivate();
        ~ObjectPoolPrivate();

        void init();

        ObjectPool *q_ptr;

        // all objects
        std::list<QObject *> objects;

        // id -> objects with same id
        QHash<QString, stdc::linked_map<QObject *, int /*NOT USED*/>> objectMap;

        struct Index {
            QString id;
            decltype(objects)::iterator it;
        };

        // object -> index
        QHash<QObject *, Index> objectIndexes;

        mutable QReadWriteLock objectListLock;

        void objectAdded(const QString &id, QObject *obj);
        void aboutToRemoveObject(const QString &id, QObject *obj);

        friend class ObjectPool;

    private:
        void _q_objectDestroyed();
    };


}

#endif // OBJECTPOOL_P_H
