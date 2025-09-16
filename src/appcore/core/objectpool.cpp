#include "objectpool.h"
#include "objectpool_p.h"

#include <utility>

#include <QDebug>
#include <QMetaMethod>

#define DISABLE_WARNING_OBJECTS_LEFT

namespace Core {

#define myWarning (qWarning().nospace() << "Core::ObjectPool::" << __func__ << "():").space()

    ObjectPoolPrivate::ObjectPoolPrivate() {
    }

    ObjectPoolPrivate::~ObjectPoolPrivate() = default;

    void ObjectPoolPrivate::init() {
    }

    void ObjectPoolPrivate::objectAdded(const QString &id, QObject *obj) {
        Q_Q(ObjectPool);
        Q_EMIT q->objectAdded(id, obj);
        connect(obj, &QObject::destroyed, this, &ObjectPoolPrivate::_q_objectDestroyed);
    }

    void ObjectPoolPrivate::aboutToRemoveObject(const QString &id, QObject *obj) {
        Q_Q(ObjectPool);
        disconnect(obj, &QObject::destroyed, this, &ObjectPoolPrivate::_q_objectDestroyed);
        Q_EMIT q->aboutToRemoveObject(id, obj);
    }

    void ObjectPoolPrivate::_q_objectDestroyed() {
        Q_Q(ObjectPool);
        q->removeObject(sender());
    }

    ObjectPool::ObjectPool(QObject *parent) : ObjectPool(*new ObjectPoolPrivate(), parent) {
    }

    ObjectPool::~ObjectPool() {
        Q_D(ObjectPool);
#ifndef DISABLE_WARNING_OBJECTS_LEFT
        if (!d->objects.empty()) {
            qDebug() << "There are" << d->objects.size() << "objects left in the object pool.";

            // Intentionally split debug info here, since in case the list contains
            // already deleted object we get at least the info about the number of objects;
            qDebug() << "The following objects left in the object pool of" << this << ":"
                     << QList<QObject *>(d->objects.begin(), d->objects.end());
        }
#endif
    }

    void ObjectPool::addObject(QObject *obj) {
        addObject({}, obj);
    }

    void ObjectPool::addObject(const QString &id, QObject *obj) {
        Q_D(ObjectPool);
        if (!obj) {
            myWarning << "trying to add null object";
            return;
        }

        {
            QWriteLocker locker(&d->objectListLock);
            auto &set = d->objectMap[id];
            if (!set.append(obj, 0).second) {
                myWarning << "trying to add duplicated object:" << id << obj;
                return;
            }

            // Add to list
            auto it = d->objects.insert(d->objects.end(), obj);

            // Add to index
            d->objectIndexes.insert(obj, {id, it});
        }

        d->objectAdded(id, obj);
    }

    void ObjectPool::removeObject(QObject *obj) {
        Q_D(ObjectPool);
        QString id;
        {
            QReadLocker locker(&d->objectListLock);

            auto it = d->objectIndexes.find(obj);
            if (it == d->objectIndexes.end()) {
                myWarning << "obj does not exist:" << obj;
                return;
            }

            id = it.value().id;
        }

        d->aboutToRemoveObject(id, obj);

        {
            QWriteLocker locker(&d->objectListLock);
            {
                auto it = d->objectMap.find(id);

                // Remove from map
                auto &set = it.value();
                set.remove(obj);
                if (set.empty()) {
                    d->objectMap.erase(it);
                }
            }

            {
                auto it = d->objectIndexes.find(obj);

                // Remove from list
                d->objects.erase(it->it);

                // Remove from indexes
                d->objectIndexes.erase(it);
            }
        }
    }

    void ObjectPool::removeObjects(const QString &id) {
        Q_D(ObjectPool);
        QList<QObject *> objs;
        {
            QReadLocker locker(&d->objectListLock);
            auto it = d->objectMap.find(id);
            if (it == d->objectMap.end()) {
                return;
            }
            objs = it->keys_qlist();
        }

        for (const auto &obj : qAsConst(objs)) {
            d->aboutToRemoveObject(id, obj);
        }

        {
            QWriteLocker locker(&d->objectListLock);
            auto it = d->objectMap.find(id);
            for (const auto &item : std::as_const(it.value())) {
                d->objectIndexes.remove(item.first);
            }
            d->objectMap.erase(it);
        }
    }

    QList<QObject *> ObjectPool::allObjects() const {
        Q_D(const ObjectPool);
        return d->objectIndexes.keys();
    }

    QReadWriteLock *ObjectPool::listLock() const {
        Q_D(const ObjectPool);
        return &d->objectListLock;
    }

    QList<QObject *> ObjectPool::getObjects(const QString &id) const {
        Q_D(const ObjectPool);
        QReadLocker locker(&d->objectListLock);
        auto it2 = d->objectMap.find(id);
        if (it2 != d->objectMap.end()) {
            return it2->keys_qlist();
        }
        return {};
    }

    QObject *ObjectPool::getFirstObject(const QString &id) const {
        Q_D(const ObjectPool);
        QReadLocker locker(&d->objectListLock);
        auto it2 = d->objectMap.find(id);
        if (it2 != d->objectMap.end()) {
            return it2->empty() ? nullptr : it2->begin()->first;
        }
        return nullptr;
    }

    ObjectPool::ObjectPool(ObjectPoolPrivate &d, QObject *parent) : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.init();
    }

}
