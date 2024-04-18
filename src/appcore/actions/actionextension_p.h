#ifndef ACTIONEXTENSION_P_H
#define ACTIONEXTENSION_P_H

#include <CoreApi/actionextension.h>

namespace Core {

    struct ActionObjectInfoData {
        QString id;
        ActionObjectInfo::Type type;
        QByteArray text;
        QByteArray commandClass;
        QList<QKeySequence> shortcuts;
        QByteArrayList categories;
        bool topLevel;
    };

    struct ActionLayoutInfoData {
        struct Entry {
            QString id;
            ActionObjectInfo::Type type;
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

        int objectCount;
        ActionObjectInfoData *objectData;

        int layoutCount;
        ActionLayoutInfoData *layoutData;

        int buildRoutineCount;
        ActionBuildRoutineData *buildRoutineData;

        static inline const ActionExtensionPrivate *get(const ActionExtension *q) {
            Q_ASSERT(q->d.data);
            return static_cast<const ActionExtensionPrivate *>(q->d.data);
        }
    };

}

#endif // ACTIONEXTENSION_P_H
