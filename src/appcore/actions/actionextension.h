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
        inline ActionObjectInfo() : data(nullptr){};
        inline bool isValid() const {
            return data != nullptr;
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

        friend class ActionExtension;
    };

    class CKAPPCORE_EXPORT ActionLayoutInfo {
    public:
        inline ActionLayoutInfo() : data(nullptr), idx(0){};
        inline bool isValid() const {
            return data != nullptr;
        };

        QString id() const;
        ActionObjectInfo::Type type() const;
        bool flat() const;

        int childCount() const;
        ActionLayoutInfo child(int index) const;

    protected:
        const void *data;
        int idx;

        friend class ActionExtension;
    };

    class CKAPPCORE_EXPORT ActionBuildRoutine {
    public:
        inline ActionBuildRoutine() : data(nullptr){};
        inline bool isValid() const {
            return data != nullptr;
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

        struct Item {
            QString id;
            ActionObjectInfo::Type type;
            bool flat;
        };
        int itemCount() const;
        Item item(int index) const;

    protected:
        const void *data;

        friend class ActionExtension;
    };

    class CKAPPCORE_EXPORT ActionExtension {
    public:
        inline bool isValid() const {
            return d.data != nullptr;
        };

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
