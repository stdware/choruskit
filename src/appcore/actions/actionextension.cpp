#include "actionextension.h"
#include "actionextension_p.h"

#include <QtCore/QCoreApplication>

namespace Core {

    QString ActionObjectInfo::id() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->objectData[idx].id;
    }

    ActionObjectInfo::Type ActionObjectInfo::type() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->objectData[idx].type;
    }

    ActionObjectInfo::Mode ActionObjectInfo::mode() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->objectData[idx].mode;
    }

    QByteArray ActionObjectInfo::text() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->objectData[idx].text;
    }

    QByteArray ActionObjectInfo::commandClass() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->objectData[idx].commandClass;
    }

    QList<QKeySequence> ActionObjectInfo::shortcuts() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->objectData[idx].shortcuts;
    }

    QByteArrayList ActionObjectInfo::categories() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->objectData[idx].categories;
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

    QString ActionLayoutInfo::id() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->layoutEntryData[idx].id;
    }

    ActionLayoutInfo::Type ActionLayoutInfo::type() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->layoutEntryData[idx].type;
    }

    int ActionLayoutInfo::childCount() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->layoutEntryData[idx].childIndexes.size();
    }

    ActionLayoutInfo ActionLayoutInfo::child(int index) const {
        if (!ext)
            return {};
        ActionLayoutInfo result;
        result.ext = ext;
        result.idx = ActionExtensionPrivate::get(ext)->layoutEntryData[idx].childIndexes[index];
        return result;
    }

    ActionBuildRoutine::Anchor ActionBuildRoutine::anchor() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->buildRoutineData->anchor;
    }

    QString ActionBuildRoutine::parent() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->buildRoutineData->parent;
    }

    QString ActionBuildRoutine::relativeTo() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->buildRoutineData->relativeTo;
    }

    int ActionBuildRoutine::itemCount() const {
        if (!ext)
            return {};
        return ActionExtensionPrivate::get(ext)->buildRoutineData->entryIndexes.size();
    }

    ActionLayoutInfo ActionBuildRoutine::item(int index) const {
        if (!ext)
            return {};
        ActionLayoutInfo result;
        result.ext = ext;
        result.idx = ActionExtensionPrivate::get(ext)->buildRoutineData->entryIndexes.at(index);
        return result;
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
        result.ext = this;
        result.idx = index;
        return result;
    }

    int ActionExtension::layoutCount() const {
        return ActionExtensionPrivate::get(this)->layoutRootCount;
    }

    ActionLayoutInfo ActionExtension::layout(int index) const {
        ActionLayoutInfo result;
        result.ext = this;
        result.idx = ActionExtensionPrivate::get(this)->layoutRootData[index];
        return result;
    }

    int ActionExtension::buildRoutineCount() const {
        return ActionExtensionPrivate::get(this)->buildRoutineCount;
    }

    ActionBuildRoutine ActionExtension::buildRoutine(int index) const {
        ActionBuildRoutine result;
        result.ext = this;
        result.idx = index;
        return result;
    }

}