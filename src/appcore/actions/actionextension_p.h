#ifndef ACTIONEXTENSION_P_H
#define ACTIONEXTENSION_P_H

#include <CoreApi/actionextension.h>

namespace Core {

    struct ActionObjectInfoData {
        QString id;
        ActionObjectInfo::Type type;
        int mode;
        QByteArray text;
        QByteArray commandClass;
        QList<QKeySequence> shortcuts;
        QByteArrayList categories;
    };

    struct ActionLayoutInfoEntry {
        QString id;
        ActionLayoutInfo::Type type;
        QVector<int> childIndexes;
    };

    struct ActionBuildRoutineData {
        ActionBuildRoutine::Anchor anchor;
        QString parent;
        QString relativeTo;
        QVector<int> entryIndexes;
    };

    struct ActionExtensionPrivate {
        QString hash;

        QString version;

        int objectCount;
        ActionObjectInfoData *objectData;

        int layoutEntryCount; // Not used
        ActionLayoutInfoEntry *layoutEntryData;

        int layoutRootCount;
        int *layoutRootData;

        int buildRoutineCount;
        ActionBuildRoutineData *buildRoutineData;

        static inline const ActionExtensionPrivate *get(const ActionExtension *q) {
            Q_ASSERT(q->d.data);
            return static_cast<const ActionExtensionPrivate *>(q->d.data);
        }
    };

}

#endif // ACTIONEXTENSION_P_H
