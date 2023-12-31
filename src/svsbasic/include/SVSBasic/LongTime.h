#ifndef LONGTIME_H
#define LONGTIME_H

#include <QMetaType>
#include <QStringList>

#include "CkSVSBasicGlobal.h"

namespace SVS {

    class CKSVSBASIC_API LongTime {
    public:
        Q_DECL_CONSTEXPR inline LongTime();
        Q_DECL_CONSTEXPR inline LongTime(int msec);
        LongTime(int minute, int second, int msec);

        Q_DECL_CONSTEXPR inline int minute() const;
        Q_DECL_CONSTEXPR inline int second() const;
        Q_DECL_CONSTEXPR inline int msec() const;
        Q_DECL_CONSTEXPR inline int totalMsec() const;

        QString toString() const;
        static LongTime fromString(const QString &s);

        Q_DECL_CONSTEXPR bool operator==(const LongTime &other) const;
        Q_DECL_CONSTEXPR bool operator!=(const LongTime &other) const;
        Q_DECL_CONSTEXPR bool operator<(const LongTime &other) const;
        Q_DECL_CONSTEXPR bool operator<=(const LongTime &other) const;
        Q_DECL_CONSTEXPR bool operator>(const LongTime &other) const;
        Q_DECL_CONSTEXPR bool operator>=(const LongTime &other) const;

        friend CKSVSBASIC_API QDebug operator<<(QDebug debug, const LongTime &lt);

    private:
        int t;
    };

    Q_DECL_CONSTEXPR LongTime::LongTime() : t(0) {
    }

    Q_DECL_CONSTEXPR LongTime::LongTime(int msec) : t(qMax(msec, 0)) {
    }

    Q_DECL_CONSTEXPR inline int LongTime::minute() const {
        return t / 60000;
    }

    Q_DECL_CONSTEXPR inline int LongTime::second() const {
        return t % 60000 / 1000;
    }

    Q_DECL_CONSTEXPR inline int LongTime::msec() const {
        return t % 1000;
    }

    Q_DECL_CONSTEXPR inline int LongTime::totalMsec() const {
        return t;
    }

    Q_DECL_CONSTEXPR bool LongTime::operator==(const LongTime &other) const {
        return t == other.t;
    }

    Q_DECL_CONSTEXPR bool LongTime::operator!=(const LongTime &other) const {
        return t != other.t;
    }

    Q_DECL_CONSTEXPR bool LongTime::operator<(const LongTime &other) const {
        return t < other.t;
    }

    Q_DECL_CONSTEXPR bool LongTime::operator<=(const LongTime &other) const {
        return t <= other.t;
    }

    Q_DECL_CONSTEXPR bool LongTime::operator>(const LongTime &other) const {
        return t > other.t;
    }

    Q_DECL_CONSTEXPR bool LongTime::operator>=(const LongTime &other) const {
        return t >= other.t;
    }

}

CKSVSBASIC_API uint qHash(const SVS::LongTime &time, uint seed);

Q_DECLARE_METATYPE(SVS::LongTime)

#endif // LONGTIME_H
