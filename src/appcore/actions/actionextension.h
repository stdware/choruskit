#ifndef ACTIONEXTENSION_H
#define ACTIONEXTENSION_H

#include <QObject>
#include <QAction>
#include <QKeySequence>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class ActionDomain;

    class ActionExtension;

    class ActionBuildRoutine;

    class CKAPPCORE_EXPORT ActionObjectInfo {
    public:
        inline ActionObjectInfo() : ext(nullptr), idx(0){};
        inline bool isNull() const {
            return ext == nullptr;
        };

        enum Type {
            Action,
            Group,
            Menu,
        };

        enum Mode {
            Plain = 0,
            Widget = 1,
            Unique = 1,
            TopLevel = 2,
        };

        QString id() const;
        Type type() const;
        Mode mode() const;
        QByteArray text() const;
        QByteArray commandClass() const;
        QList<QKeySequence> shortcuts() const;
        QByteArrayList categories() const;

        static QString translatedText(const QByteArray &text, bool *ok = nullptr);
        static QString translatedCommandClass(const QByteArray &commandClass, bool *ok = nullptr);
        static QString translatedCategory(const QByteArray &category, bool *ok = nullptr);

    protected:
        const ActionExtension *ext;
        int idx;

        friend class ActionExtension;
        friend class ActionDomain;
    };

    class CKAPPCORE_EXPORT ActionLayoutInfo {
    public:
        inline ActionLayoutInfo() : ext(nullptr), idx(0){};
        inline bool isNull() const {
            return ext == nullptr;
        };

        enum TypeFlags {
            TerminalFlag = 1,    // leaf node
            GroupFlag = 2,       // group of nodes
            PlaceHolderFlag = 4, // placeholder node
        };

        enum Type {
            Action = 0x100 | TerminalFlag,
            Group = 0x200 | GroupFlag,
            Menu = 0x400,
            ExpandedMenu = 0x400 | GroupFlag,
            Separator = 0x800 | TerminalFlag | PlaceHolderFlag,
            Stretch = 0x1000 | TerminalFlag | PlaceHolderFlag,
        };

        QString id() const;
        Type type() const;

        int childCount() const;
        ActionLayoutInfo child(int index) const;

    protected:
        const ActionExtension *ext;
        int idx;

        friend class ActionExtension;
        friend class ActionBuildRoutine;
        friend class ActionDomain;
    };

    class CKAPPCORE_EXPORT ActionBuildRoutine {
    public:
        inline ActionBuildRoutine() : ext(nullptr), idx(0){};
        inline bool isNull() const {
            return ext == nullptr;
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

        int itemCount() const;
        ActionLayoutInfo item(int index) const;

    protected:
        const ActionExtension *ext;
        int idx;

        friend class ActionExtension;
        friend class ActionDomain;
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
