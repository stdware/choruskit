#ifndef MUSICTIMELINEPRIVATE_H
#define MUSICTIMELINEPRIVATE_H

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the ChorusKit API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

#include <QMap>
#include <QSet>

#include <SVSBasic/MusicTimeline.h>

namespace SVS {

    class CKSVSBASIC_API MusicTimelinePrivate {
        Q_DECLARE_PUBLIC(MusicTimeline)
    public:
        MusicTimelinePrivate();
        virtual ~MusicTimelinePrivate();

        void init();

        MusicTimeline *q_ptr;

        const int resolution = 480;

        QMap<int, MusicTimeSignature> timeSignatureMap; // bar position -> time signature
        QMap<int, int> measureMap;                      // tick position -> bar position
        QMap<int, int> revMeasureMap;                   // bar position -> tick position
        QMap<int, double> tempoMap;                     // tick position -> tempo
        QMap<int, double> msecSumMap;                   // tick position -> msecs in total
        QMap<double, int> revMsecSumMap;                // msecs in total -> tick position

        mutable QSet<PersistentMusicTimeData *> mbtCachedMusicTimes;
        mutable QSet<PersistentMusicTimeData *> msecCachedMusicTimes;

        void updateMeasureMap(int barFrom);
        void updateMsecSumMap(int tickFrom);

        void timeSignatureChanged();
        void tempoChanged();

    public:
        int findNearestTickWithTimeSignature(int tick) const;
        double findNearestMsecWithTempo(double msec) const;

        friend class PersistentMusicTime;
    };

}

#endif // MUSICTIMELINEPRIVATE_H
