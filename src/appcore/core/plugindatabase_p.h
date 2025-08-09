#ifndef CHORUSKIT_PLUGINDATABASE_P_H
#define CHORUSKIT_PLUGINDATABASE_P_H

#include <CoreApi/plugindatabase.h>

#include <QtQmlIntegration/qqmlintegration.h>

namespace Core {
    struct PluginDatabaseForeign {
        Q_GADGET
        QML_FOREIGN(PluginDatabase)
        QML_SINGLETON
        QML_NAMED_ELEMENT(PluginDatabase)
    public:

        static inline PluginDatabase *create(QQmlEngine *, QJSEngine *engine) {
            return PluginDatabase::instance();
        }
    };
}

#endif //CHORUSKIT_PLUGINDATABASE_P_H
