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

    private:
        const void *data;

        friend class ActionExtension;
    };

    class CKAPPCORE_EXPORT ActionLayout {
    public:
        inline ActionLayout() : data(nullptr), idx(0){};

        QString id() const;
        ActionObjectInfo::Type type() const;
        bool flat() const;

        int childCount() const;
        ActionLayout child(int index) const;

    private:
        const void *data;
        int idx;

        friend class ActionExtension;
        friend class ActionDomain;
    };

    class CKAPPCORE_EXPORT ActionBuildRoutine {
    public:
        inline ActionBuildRoutine() : data(nullptr){};

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

    private:
        const void *data;

        friend class ActionExtension;
    };

    class CKAPPCORE_EXPORT ActionExtension {
    public:
        QString hash() const;

        QString version() const;

        int objectCount() const;
        ActionObjectInfo object(int index) const;

        int layoutCount() const;
        ActionLayout layout(int index) const;

        int buildRoutineCount() const;
        ActionBuildRoutine buildRoutine(int index) const;

        struct {
            const void *data;
        } d;
    };

}

#define CK_GET_ACTION_EXTENSION(name)                                                              \
    []() {                                                                                         \
        extern const Core::ActionExtension *QT_MANGLE_NAMESPACE(                                   \
            ckGetStaticActionExtension_##name)();                                                  \
        return QT_MANGLE_NAMESPACE(ckGetStaticActionExtension_##name)();                           \
    }()

#endif // ACTIONEXTENSION_H
