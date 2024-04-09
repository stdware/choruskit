#ifndef OBJECTPOOL_P_H
#define OBJECTPOOL_P_H

#include <QMCore/qmchronoset.h>

#include <CoreApi/objectpool.h>

namespace Core {

    class ObjectPoolPrivate : public QObject {
    public:
        ObjectPoolPrivate(ObjectPool *q);
        ~ObjectPoolPrivate();

        ObjectPool *q;

        // all objects
        std::list<QObject *> objects;

        // id -> objects with same id
        QHash<QString, QMChronoSet<QObject *>> objectMap;

        struct Index {
            QString id;
            decltype(objects)::iterator it;
        };

        // object -> index
        QHash<QObject *, Index> objectIndexes;

        mutable QReadWriteLock objectListLock;

        QHash<QString, QVariant> globalAttributeMap;

        mutable QReadWriteLock globalAttributeLock;

        struct Checkable {
            QString id;
            QObject *obj;
            bool reverse;
        };

        QHash<QObject *, Checkable> checkableMap1;
        QHash<QString, Checkable> checkableMap2;

        void objectAdded(const QString &id, QObject *obj);
        void aboutToRemoveObject(const QString &id, QObject *obj);

        friend class ObjectPool;

    private:
        void _q_objectDestroyed();
    };


}

#endif // OBJECTPOOL_P_H
