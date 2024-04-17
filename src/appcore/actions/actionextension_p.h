#ifndef ACTIONEXTENSION_P_H
#define ACTIONEXTENSION_P_H

#include <CoreApi/actionextension.h>

namespace Core {

    struct ActionItemInfoData {
        QString id;
        ActionItemInfo::Type type;
        QByteArray text;
        QByteArray commandClass;
        QList<QKeySequence> shortcuts;
        QByteArrayList categories;
        bool topLevel;
    };

    struct ActionLayoutData {
        struct Entry {
            QString id;
            ActionItemInfo::Type type;
            bool flat;
            QVector<int> childIndexes;
        };
        QVector<Entry> entryData;
    };

    struct ActionBuildRoutineData {
        ActionBuildRoutine::Anchor anchor;
        QString parent;
        QString relativeTo;
        QVector<ActionBuildRoutine::Item> items;
    };

    struct ActionExtensionPrivate {
        QString hash;
        
        QString version;

        int itemCount;
        ActionItemInfoData *itemData;

        int layoutCount;
        ActionLayoutData *layoutData;

        int buildRoutineCount;
        ActionBuildRoutineData *buildRoutineData;

        static inline const ActionExtensionPrivate *get(const ActionExtension *q) {
            Q_ASSERT(q->d.data);
            return static_cast<const ActionExtensionPrivate *>(q->d.data);
        }
    };

}

#endif // ACTIONEXTENSION_P_H
