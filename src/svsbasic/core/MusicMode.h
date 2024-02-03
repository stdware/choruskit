#ifndef MUSICMODE_H
#define MUSICMODE_H

#include <QObject>

#include <SVSBasic/CkSVSBasicGlobal.h>

namespace SVS {

    class CKSVSBASIC_API MusicMode {
        Q_GADGET
    public:
        enum Type {
            Ionian,
            Dorian,
            Phrygian,
            Lydian,
            Mixolydian,
            Aeolian,
            Locrian,
        };
        Q_ENUM(Type)

        MusicMode();
        MusicMode(Type type, int offset = 0);
        MusicMode(const QList<int> &keys, int offset = 0);

        Q_DECL_CONSTEXPR inline int offset() const;
        Q_DECL_CONSTEXPR inline void setOffset(int offset);

        Q_DECL_CONSTEXPR inline bool isTonic(int key) const;

    private:
        union {
            struct {
                quint8 c : 1;
                quint8 c_ : 1;
                quint8 d : 1;
                quint8 d_ : 1;
                quint8 e : 1;
                quint8 f : 1;
                quint8 f_ : 1;
                quint8 g : 1;
                quint8 g_ : 1;
                quint8 a : 1;
                quint8 a_ : 1;
                quint8 b : 1;
            };
            quint16 val;
        } m_keys;
        quint8 m_offset;
    };

    Q_DECL_CONSTEXPR int MusicMode::offset() const {
        return m_offset;
    }

    Q_DECL_CONSTEXPR void MusicMode::setOffset(int offset) {
        m_offset = offset % 12;
    }

    Q_DECL_CONSTEXPR bool MusicMode::isTonic(int key) const {
        return m_keys.val & (1 << key % 12);
    }

}

Q_DECLARE_METATYPE(SVS::MusicMode)

#endif // MUSICMODE_H
