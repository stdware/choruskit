#ifndef ACTIONSPEC_H
#define ACTIONSPEC_H

#include <QIcon>
#include <QKeySequence>
#include <QObject>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class ActionSpec;

    class CKAPPCORE_EXPORT ActionIconSpec {
    public:
        explicit ActionIconSpec(ActionSpec *spec);
        inline explicit ActionIconSpec(const QString &fileName);
        inline ActionIconSpec();

        inline bool isFile() const;
        inline QString data() const;

        QIcon icon() const;

    private:
        bool m_file;
        QString m_data;
    };

    inline ActionIconSpec::ActionIconSpec(const QString &fileName)
        : m_file(false), m_data(fileName) {
    }

    inline ActionIconSpec::ActionIconSpec() : m_file(false) {
    }

    inline bool ActionIconSpec::isFile() const {
        return m_file;
    }

    inline QString ActionIconSpec::data() const {
        return m_data;
    }

    class ActionSpecPrivate;

    class CKAPPCORE_EXPORT ActionSpec : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionSpec)
    public:
        explicit ActionSpec(const QString &id, QObject *parent = nullptr);
        ~ActionSpec();

    public:
        QString id() const;

        QString commandName() const;
        void setCommandName(const QString &name);

        QString commandDisplayName() const;
        void setCommandDisplayName(const QString &displayName);

        QList<QKeySequence> shortcuts() const;
        void setShortcuts(const QList<QKeySequence> &shortcuts);
        QList<QKeySequence> cachedShortcuts() const;

        QIcon icon() const;
        void setIcon(const QIcon &icon);
        QIcon cachedIcon() const;

    Q_SIGNALS:
        void shortcutsChanged();
        void iconChanged();

    protected:
        ActionSpec(ActionSpecPrivate &d, const QString &id, QObject *parent = nullptr);

        QScopedPointer<ActionSpecPrivate> d_ptr;
    };

}

#endif // ACTIONSPEC_H
