#include "actionextension.h"
#include "actionextension_p.h"

#include <QtCore/QCoreApplication>

namespace Core {

    QString ActionItemInfo::id() const {
        return static_cast<const ActionItemInfoData *>(data)->id;
    }

    ActionItemInfo::Type ActionItemInfo::type() const {
        return static_cast<const ActionItemInfoData *>(data)->type;
    }

    QByteArray ActionItemInfo::text() const {
        return static_cast<const ActionItemInfoData *>(data)->text;
    }

    QByteArray ActionItemInfo::commandClass() const {
        return static_cast<const ActionItemInfoData *>(data)->commandClass;
    }

    QList<QKeySequence> ActionItemInfo::shortcuts() const {
        return static_cast<const ActionItemInfoData *>(data)->shortcuts;
    }

    QByteArrayList ActionItemInfo::categories() const {
        return static_cast<const ActionItemInfoData *>(data)->categories;
    }

    bool ActionItemInfo::topLevel() const {
        return static_cast<const ActionItemInfoData *>(data)->topLevel;
    }

    QString ActionItemInfo::translatedText(const QByteArray &text) {
        return QCoreApplication::translate("ChorusKit::ActionText", text);
    }

    QString ActionItemInfo::translatedCommandClass(const QByteArray &commandClass) {
        return QCoreApplication::translate("ChorusKit::ActionCommandClass", commandClass);
    }

    QString ActionItemInfo::translatedCategory(const QByteArray &category) {
        return QCoreApplication::translate("ChorusKit::ActionCategory", category);
    }

    QString ActionLayout::id() const {
        return static_cast<const ActionLayoutData *>(data)->entryData[idx].id;
    }

    ActionItemInfo::Type ActionLayout::type() const {
        return static_cast<const ActionLayoutData *>(data)->entryData[idx].type;
    }

    bool ActionLayout::flat() const {
        return static_cast<const ActionLayoutData *>(data)->entryData[idx].flat;
    }

    int ActionLayout::childCount() const {
        return static_cast<const ActionLayoutData *>(data)->entryData[idx].childIndexes.size();
    }

    ActionLayout ActionLayout::child(int index) const {
        ActionLayout result = *this;
        result.idx =
            static_cast<const ActionLayoutData *>(data)->entryData[idx].childIndexes[index];
        return result;
    }

    ActionBuildRoutine::Anchor ActionBuildRoutine::anchor() const {
        return static_cast<const ActionBuildRoutineData *>(data)->anchor;
    }

    QString ActionBuildRoutine::parent() const {
        return static_cast<const ActionBuildRoutineData *>(data)->parent;
    }

    QString ActionBuildRoutine::relativeTo() const {
        return static_cast<const ActionBuildRoutineData *>(data)->relativeTo;
    }

    int ActionBuildRoutine::itemCount() const {
        return static_cast<const ActionBuildRoutineData *>(data)->items.size();
    }

    ActionBuildRoutine::Item ActionBuildRoutine::item(int index) const {
        return static_cast<const ActionBuildRoutineData *>(data)->items[index];
    }

    QString ActionExtension::hash() const {
        return ActionExtensionPrivate::get(this)->hash;
    }

    QString ActionExtension::version() const {
        return ActionExtensionPrivate::get(this)->version;
    }

    int ActionExtension::itemCount() const {
        return ActionExtensionPrivate::get(this)->itemCount;
    }

    ActionItemInfo ActionExtension::item(int index) const {
        ActionItemInfo result;
        result.data = &ActionExtensionPrivate::get(this)->itemData[index];
        return result;
    }

    int ActionExtension::layoutCount() const {
        return ActionExtensionPrivate::get(this)->layoutCount;
    }

    ActionLayout ActionExtension::layout(int index) const {
        ActionLayout result;
        result.data = &ActionExtensionPrivate::get(this)->layoutData[index];
        result.idx = index;
        return result;
    }

    int ActionExtension::buildRoutineCount() const {
        return ActionExtensionPrivate::get(this)->buildRoutineCount;
    }

    ActionBuildRoutine ActionExtension::buildRoutine(int index) const {
        ActionBuildRoutine result;
        result.data = &ActionExtensionPrivate::get(this)->buildRoutineData[index];
        return result;
    }

}