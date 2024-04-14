#ifndef ACTIONEXTENSION_H
#define ACTIONEXTENSION_H

#include <QObject>
#include <QAction>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    struct ActionExtension;

    class CKAPPCORE_EXPORT ActionMetaItem {
    public:
        inline ActionMetaItem() : data(nullptr){};

        enum Type {
            Action,
            Group,
            Menu,
            Separator,
            Stretch,
        };

        QString id() const;
        Type type() const;
        QString text() const;
        QString commandClass() const;
        QList<QKeySequence> shortcuts() const;
        QStringList category() const;
        bool topLevel() const;

    private:
        const void *data;

        friend class ActionExtension;
    };

    class CKAPPCORE_EXPORT ActionMetaLayout {
    public:
        inline ActionMetaLayout() : data(nullptr), idx(0){};

        QString id() const;
        ActionMetaItem::Type type() const;
        bool flat() const;

        int childCount() const;
        ActionMetaLayout child(int index) const;

    private:
        const void *data;
        int idx;

        friend class ActionExtension;
    };

    class CKAPPCORE_EXPORT ActionMetaRoutine {
    public:
        inline ActionMetaRoutine() : data(nullptr){};

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
            bool flat;
        };
        int itemCount() const;
        Item item(int index) const;

    private:
        const void *data;

        friend class ActionExtension;
    };

    struct CKAPPCORE_EXPORT ActionExtension {
        QByteArray hash() const;

        QString version() const;

        int itemCount() const;
        ActionMetaItem item(int index) const;

        int layoutCount() const;
        ActionMetaLayout layout(int index) const;

        int routineCount() const;
        ActionMetaRoutine routine(int index) const;

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
