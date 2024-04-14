#include "actionextension_p.h"
#include "actionextension.h"

namespace Core {

    QString ActionMetaItem::id() const {
        return static_cast<const ActionMetaItemData *>(data)->id;
    }

    ActionMetaItem::Type ActionMetaItem::type() const {
        return static_cast<const ActionMetaItemData *>(data)->type;
    }

    QString ActionMetaItem::text() const {
        return static_cast<const ActionMetaItemData *>(data)->text();
    }

    QString ActionMetaItem::commandClass() const {
        return static_cast<const ActionMetaItemData *>(data)->commandClass();
    }

    QList<QKeySequence> ActionMetaItem::shortcuts() const {
        return static_cast<const ActionMetaItemData *>(data)->shortcuts;
    }

    QStringList ActionMetaItem::category() const {
        return static_cast<const ActionMetaItemData *>(data)->category();
    }

    bool ActionMetaItem::topLevel() const {
        return static_cast<const ActionMetaItemData *>(data)->topLevel;
    }

    QString ActionMetaLayout::id() const {
        return static_cast<const ActionMetaLayoutData *>(data)->entryData[idx].id;
    }

    ActionMetaItem::Type ActionMetaLayout::type() const {
        return static_cast<const ActionMetaLayoutData *>(data)->entryData[idx].type;
    }

    bool ActionMetaLayout::flat() const {
        return static_cast<const ActionMetaLayoutData *>(data)->entryData[idx].flat;
    }

    int ActionMetaLayout::childCount() const {
        return static_cast<const ActionMetaLayoutData *>(data)->entryData[idx].childCount;
    }

    ActionMetaLayout ActionMetaLayout::child(int index) const {
        ActionMetaLayout result = *this;
        result.idx = static_cast<const ActionMetaLayoutData *>(data)
                         ->entryData[idx]
                         .childrenIndexData[index];
        return result;
    }

    ActionMetaRoutine::Anchor ActionMetaRoutine::anchor() const {
        return static_cast<const ActionMetaRoutineData*>(data)->anchor;
    }

    QString ActionMetaRoutine::parent() const {
        return static_cast<const ActionMetaRoutineData*>(data)->parent;
    }

    QString ActionMetaRoutine::relativeTo() const {
        return static_cast<const ActionMetaRoutineData*>(data)->relativeTo;
    }

    int ActionMetaRoutine::itemCount() const {
        return static_cast<const ActionMetaRoutineData*>(data)->itemCount;
    }

    ActionMetaRoutine::Item ActionMetaRoutine::item(int index) const {
        return static_cast<const ActionMetaRoutineData*>(data)->itemData[index];
    }

    QByteArray ActionExtension::hash() const {
        return ActionExtensionPrivate::get(this)->hash;
    }

    QString ActionExtension::version() const {
        return ActionExtensionPrivate::get(this)->version;
    }

    int ActionExtension::itemCount() const {
        return ActionExtensionPrivate::get(this)->itemCount;
    }

    ActionMetaItem ActionExtension::item(int index) const {
        ActionMetaItem result;
        result.data = &ActionExtensionPrivate::get(this)->itemData[index];
        return result;
    }

    int ActionExtension::layoutCount() const {
        return ActionExtensionPrivate::get(this)->layoutCount;
    }

    ActionMetaLayout ActionExtension::layout(int index) const {
        ActionMetaLayout result;
        result.data = &ActionExtensionPrivate::get(this)->layoutData[index];
        result.idx = index;
        return result;
    }

    int ActionExtension::routineCount() const {
        return ActionExtensionPrivate::get(this)->routineCount;
    }

    ActionMetaRoutine ActionExtension::routine(int index) const {
        ActionMetaRoutine result;
        result.data = &ActionExtensionPrivate::get(this)->routineData[index];
        return result;
    }

}