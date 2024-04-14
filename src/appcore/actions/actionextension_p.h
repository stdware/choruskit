#ifndef ACTIONEXTENSION_P_H
#define ACTIONEXTENSION_P_H

#include <CoreApi/actionextension.h>

namespace Core {

    struct ActionMetaItemData {
        using StringGetter = QString (*)();
        using StringListGetter = QStringList (*)();

        QString id;
        ActionMetaItem::Type type;
        StringGetter text;
        StringGetter commandClass;
        QList<QKeySequence> shortcuts;
        StringListGetter category;
        bool topLevel;
    };

    struct ActionMetaLayoutData {
        struct Entry {
            QString id;
            ActionMetaItem::Type type;
            bool flat;
            int childCount;
            int *childrenIndexData;
        };

        Entry *entryData;
    };

    struct ActionMetaRoutineData {
        ActionMetaRoutine::Anchor anchor;
        QString parent;
        QString relativeTo;

        int itemCount;
        ActionMetaRoutine::Item *itemData;
    };

    struct ActionExtensionPrivate {
        QByteArray hash;
        
        QString version;

        int itemCount;
        ActionMetaItemData *itemData;

        int layoutCount;
        ActionMetaLayoutData *layoutData;

        int routineCount;
        ActionMetaRoutineData *routineData;

        static inline const ActionExtensionPrivate *get(const ActionExtension *q) {
            return static_cast<const ActionExtensionPrivate *>(q->d.data);
        }
    };

}

#endif // ACTIONEXTENSION_P_H
