#ifndef MUSICTIME_P_H
#define MUSICTIME_P_H

#include <QPointer>

#include "MusicTime.h"

namespace SVSBase {

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

    class CKSVSBASE_API PersistentMusicTimeData {
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
