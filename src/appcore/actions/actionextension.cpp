#include "actionextension.h"
#include "actionextension_p.h"

#include <QtCore/QCoreApplication>

namespace Core {

    QString ActionObjectInfo::id() const {
        Q_ASSERT(data);
        return static_cast<const ActionObjectInfoData *>(data)->id;
    }

    ActionObjectInfo::Type ActionObjectInfo::type() const {
        Q_ASSERT(data);
        return static_cast<const ActionObjectInfoData *>(data)->type;
    }

    QByteArray ActionObjectInfo::text() const {
        Q_ASSERT(data);
        return static_cast<const ActionObjectInfoData *>(data)->text;
    }

    QByteArray ActionObjectInfo::commandClass() const {
        Q_ASSERT(data);
        return static_cast<const ActionObjectInfoData *>(data)->commandClass;
    }

    QList<QKeySequence> ActionObjectInfo::shortcuts() const {
        Q_ASSERT(data);
        return static_cast<const ActionObjectInfoData *>(data)->shortcuts;
    }

    QByteArrayList ActionObjectInfo::categories() const {
        Q_ASSERT(data);
        return static_cast<const ActionObjectInfoData *>(data)->categories;
    }

    bool ActionObjectInfo::topLevel() const {
        Q_ASSERT(data);
        return static_cast<const ActionObjectInfoData *>(data)->topLevel;
    }

    QString ActionObjectInfo::translatedText(const QByteArray &text) {
        return QCoreApplication::translate("ChorusKit::ActionText", text);
    }

    QString ActionObjectInfo::translatedCommandClass(const QByteArray &commandClass) {
        return QCoreApplication::translate("ChorusKit::ActionCommandClass", commandClass);
    }

    QString ActionObjectInfo::translatedCategory(const QByteArray &category) {
        return QCoreApplication::translate("ChorusKit::ActionCategory", category);
    }

    const void *ActionObjectInfo::sharedNullData() {
        static ActionObjectInfoData data{{}, {}, {}, {}, {}, {}, {}};
        return &data;
    }

    QString ActionLayoutInfo::id() const {
        Q_ASSERT(data);
        return static_cast<const ActionLayoutInfoData *>(data)->entryData[idx].id;
    }

    ActionObjectInfo::Type ActionLayoutInfo::type() const {
        Q_ASSERT(data);
        return static_cast<const ActionLayoutInfoData *>(data)->entryData[idx].type;
    }

    bool ActionLayoutInfo::flat() const {
        Q_ASSERT(data);
        return static_cast<const ActionLayoutInfoData *>(data)->entryData[idx].flat;
    }

    int ActionLayoutInfo::childCount() const {
        Q_ASSERT(data);
        return static_cast<const ActionLayoutInfoData *>(data)->entryData[idx].childIndexes.size();
    }

    ActionLayoutInfo ActionLayoutInfo::child(int index) const {
        Q_ASSERT(data);
        ActionLayoutInfo result = *this;
        result.idx =
            static_cast<const ActionLayoutInfoData *>(data)->entryData[idx].childIndexes[index];
        return result;
    }

    const void *ActionLayoutInfo::sharedNullData() {
        static ActionLayoutInfoData data{{{{}, {}, {}, {}}}};
        return &data;
    }

    ActionBuildRoutine::Anchor ActionBuildRoutine::anchor() const {
        Q_ASSERT(data);
        return static_cast<const ActionBuildRoutineData *>(data)->anchor;
    }

    QString ActionBuildRoutine::parent() const {
        Q_ASSERT(data);
        return static_cast<const ActionBuildRoutineData *>(data)->parent;
    }

    QString ActionBuildRoutine::relativeTo() const {
        Q_ASSERT(data);
        return static_cast<const ActionBuildRoutineData *>(data)->relativeTo;
    }

    int ActionBuildRoutine::itemCount() const {
        Q_ASSERT(data);
        return static_cast<const ActionBuildRoutineData *>(data)->items.size();
    }

    ActionBuildRoutine::Item ActionBuildRoutine::item(int index) const {
        Q_ASSERT(data);
        return static_cast<const ActionBuildRoutineData *>(data)->items[index];
    }

    const void *ActionBuildRoutine::sharedNullData() {
        static ActionBuildRoutineData data{{}, {}, {}, {}};
        return &data;
    }

    QString ActionExtension::hash() const {
        return ActionExtensionPrivate::get(this)->hash;
    }

    QString ActionExtension::version() const {
        return ActionExtensionPrivate::get(this)->version;
    }

    int ActionExtension::objectCount() const {
        return ActionExtensionPrivate::get(this)->objectCount;
    }

    ActionObjectInfo ActionExtension::object(int index) const {
        ActionObjectInfo result;
        result.data = &ActionExtensionPrivate::get(this)->objectData[index];
        return result;
    }

    int ActionExtension::layoutCount() const {
        return ActionExtensionPrivate::get(this)->layoutCount;
    }

    ActionLayoutInfo ActionExtension::layout(int index) const {
        ActionLayoutInfo result;
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