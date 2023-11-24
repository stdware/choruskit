#ifndef MUSICTIME_P_H
#define MUSICTIME_P_H

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the ChorusKit API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

#include <QPointer>

#include <SVSBasic/MusicTime.h>

namespace SVS {

    class MusicTimelinePrivate;

    struct MusicTimeCache {
        MusicTimeCache() : measure(-1), beat(0), tick(0), msec(-1) {
        }

        inline bool isMbtNull() const {
            return measure < 0;
        }

        inline bool isMsecNull() const {
            return msec < 0;
        }

        inline void clearMbt() {
            measure = -1;
        }

        inline void clearMsec() {
            msec = -1;
        }

        int measure;
        int beat;
        int tick;
        double msec;
    };

    class PersistentMusicTimeData {
    public:
        PersistentMusicTimeData(const MusicTimeline *timeline, const MusicTimelinePrivate *td, int totalTick);
        ~PersistentMusicTimeData();

        const MusicTimeline *timeline;
        const MusicTimelinePrivate *td;

        int totalTick;
        MusicTimeCache cache;

        void ensureMbtCached();
        void ensureMsecCached();
    };

}

#endif // MUSICTIME_P_H
