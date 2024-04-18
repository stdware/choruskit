#ifndef ACTIONEXTENSION_H
#define ACTIONEXTENSION_H

#include <QObject>
#include <QAction>
#include <QKeySequence>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class ActionDomain;

    class ActionExtension;

    class CKAPPCORE_EXPORT ActionObjectInfo {
    public:
        inline ActionObjectInfo() : data(sharedNullData()){};
        inline bool isNull() const {
            return data != sharedNullData();
        };

        enum Type {
            Action,
            Group,
            Menu,
            Separator,
            Stretch,
        };

        QString id() const;
        Type type() const;
        QByteArray text() const;
        QByteArray commandClass() const;
        QList<QKeySequence> shortcuts() const;
        QByteArrayList categories() const;
        bool topLevel() const;

        static QString translatedText(const QByteArray &text);
        static QString translatedCommandClass(const QByteArray &commandClass);
        static QString translatedCategory(const QByteArray &category);

    protected:
        const void *data;

        static const void *sharedNullData();

        friend class ActionExtension;
    };

    class CKAPPCORE_EXPORT ActionLayoutInfo {
    public:
        inline ActionLayoutInfo() : data(sharedNullData()), idx(0){};
        inline bool isNull() const {
            return data != sharedNullData();
        };

        QString id() const;
        ActionObjectInfo::Type type() const;
        bool flat() const;

        int childCount() const;
        ActionLayoutInfo child(int index) const;

    protected:
        const void *data;
        int idx;

        static const void *sharedNullData();

        friend class ActionExtension;
    };

    class CKAPPCORE_EXPORT ActionBuildRoutine {
    public:
        inline ActionBuildRoutine() : data(sharedNullData()){};
        inline bool isNull() const {
            return data != sharedNullData();
        };

        enum Anchor {
            Last,
            First,
            After,
            Before,
        };

        Anchor anchor() const;
        QString parent() const;
        QString relativeTo() const;

        class Item {
        public:
            Item(const QString &id = {}, ActionObjectInfo::Type type = ActionObjectInfo::Action,
                 bool flat = false)
                : m_id(id), m_type(type), m_flat(flat) {
            }
            inline QString id() const {
                return m_id;
            }
            Q_CONSTEXPR ActionObjectInfo::Type type() const {
                return m_type;
            }
            Q_CONSTEXPR bool flat() const {
                return m_flat;
            }
        private:
            QString m_id;
            ActionObjectInfo::Type m_type;
            bool m_flat;
        };
        int itemCount() const;
        Item item(int index) const;

    protected:
        const void *data;

        static const void *sharedNullData();

        friend class ActionExtension;
    };

    class CKAPPCORE_EXPORT ActionExtension {
    public:
        QString hash() const;

        QString version() const;

        int objectCount() const;
        ActionObjectInfo object(int index) const;

        int layoutCount() const;
        ActionLayoutInfo layout(int index) const;

        int buildRoutineCount() const;
        ActionBuildRoutine buildRoutine(int index) const;

        struct {
            const void *data;
        } d;
    };

}

#define CK_STATIC_ACTION_EXTENSION(name)                                                           \
    []() {                                                                                         \
        extern const Core::ActionExtension *QT_MANGLE_NAMESPACE(                                   \
            ckGetStaticActionExtension_##name)();                                                  \
        return QT_MANGLE_NAMESPACE(ckGetStaticActionExtension_##name)();                           \
    }()

#define CK_STATIC_ACTION_EXTENSION_GETTER(name, func)                                              \
    static inline const Core::ActionExtension *func() {                                            \
        return CK_STATIC_ACTION_EXTENSION(name);                                                   \
    }

#endif // ACTIONEXTENSION_H
