#include "generator.h"

#include <QSet>

void error(const char *msg);

template <template <class> class Array, class T>
static QString joinNumbers(const Array<T> &arr, const QString &glue) {
    QStringList list;
    list.reserve(arr.size());
    for (const auto &item : arr) {
        list.append(QString::number(item));
    }
    return list.join(glue);
}

Generator::Generator(FILE *out, const QByteArray &inputFileName, const QByteArray &identifier,
                     const ActionExtensionMessage &message)
    : out(out), inputFileName(inputFileName), identifier(identifier), msg(message) {
}

static QByteArray escapeString(const QByteArray &bytes) {
    QByteArray res;
    res.reserve(bytes.size());
    for (const auto &ch : bytes) {
        if (ch >= 32 && ch <= 126) {
            res += ch;
            continue;
        }
        switch (ch) {
            case '\\':
                res += R"(\\)";
                continue;
            case '\'':
                res += R"(\')";
                continue;
            case '\"':
                res += R"(\")";
                continue;
            default:
                break;
        }
        QString hexStr = QString::number(static_cast<unsigned char>(ch), 16).toUpper();
        if (hexStr.length() < 2) {
            hexStr.prepend(QChar('0'));
        }
        res += "\\x";
        res += hexStr.toLatin1();
    }
    return res;
}

static void generateObjects(FILE *out, const QVector<ActionObjectInfoMessage> &objects) {
    int i = 0;
    for (const auto &item : std::as_const(objects)) {
        fprintf(out, "        {\n");
        fprintf(out, "            // index %d\n", i++);
        fprintf(out, "            // id\n");
        fprintf(out, "            QStringLiteral(\"%s\"),\n",
                escapeString(item.id.toLocal8Bit()).data());
        fprintf(out, "            // type\n");
        fprintf(out, "            ActionObjectInfo::%s,\n", item.typeToken.toLocal8Bit().data());
        fprintf(out, "            // shape\n");
        fprintf(out, "            ActionObjectInfo::%s,\n", item.modeToken.toLocal8Bit().data());
        fprintf(out, "            // text\n");
        fprintf(out, "            QByteArrayLiteral(\"%s\"),\n",
                escapeString(item.text.toLocal8Bit()).data());
        fprintf(out, "            // commandClass\n");
        if (item.commandClass.isEmpty()) {
            fprintf(out, "            QByteArray(),\n");
        } else {
            fprintf(out, "            QByteArrayLiteral(\"%s\"),\n",
                    escapeString(item.commandClass.toLocal8Bit()).data());
        }
        fprintf(out, "            // shortcuts\n");
        fprintf(out, "            {\n");
        for (const auto &subItem : std::as_const(item.shortcutTokens)) {
            fprintf(out, "                QKeySequence(QStringLiteral(\"%s\")),\n",
                    subItem.trimmed().toLocal8Bit().data());
        }
        fprintf(out, "            },\n");
        fprintf(out, "            // categories\n");
        fprintf(out, "            {\n");
        for (const auto &subItem : std::as_const(item.categories)) {
            fprintf(out, "                QByteArrayLiteral(\"%s\"),\n",
                    escapeString(subItem.toLocal8Bit()).data());
        }
        fprintf(out, "            },\n");
        fprintf(out, "        },\n");
    }
}

static void generateLayouts(FILE *out, const QVector<ActionLayoutEntryMessage> &layouts) {
    int i = 0;
    for (const auto &subItem : std::as_const(layouts)) {
        fprintf(out, "        {\n");
        fprintf(out, "            // index %d\n", i++);
        fprintf(out, "            // id\n");
        if (subItem.id.isEmpty()) {
            fprintf(out, "            QString(),\n");
        } else {
            fprintf(out, "            QStringLiteral(\"%s\"),\n",
                    escapeString(subItem.id.toLocal8Bit()).data());
        }
        fprintf(out, "            // type\n");
        fprintf(out, "            ActionLayoutInfo::%s,\n", subItem.typeToken.toLocal8Bit().data());
        fprintf(out, "            // childIndexes\n");
        fprintf(out, "            {%s},\n",
                joinNumbers(subItem.childIndexes, QStringLiteral(", ")).toLocal8Bit().data());
        fprintf(out, "        },\n");
    }
}

static void generateBuildRoutines(FILE *out, const QVector<ActionBuildRoutineMessage> &routines) {
    int i = 0;
    for (const auto &item : std::as_const(routines)) {
        fprintf(out, "        {\n");
        fprintf(out, "            // index %d\n", i++);
        fprintf(out, "            // anchorToken\n");
        fprintf(out, "            ActionBuildRoutine::%s,\n",
                item.anchorToken.toLocal8Bit().data());
        fprintf(out, "            // parent\n");
        fprintf(out, "            QStringLiteral(\"%s\"),\n",
                escapeString(item.parent.toLocal8Bit()).data());
        fprintf(out, "            // relativeTo\n");
        if (item.relativeTo.isEmpty()) {
            fprintf(out, "            QString(),\n");
        } else {
            fprintf(out, "            QStringLiteral(\"%s\"),\n",
                    escapeString(item.relativeTo.toLocal8Bit()).data());
        }

        fprintf(out, "            // entryIndexes\n");
        fprintf(out, "            {%s},\n",
                joinNumbers(item.entryIndexes, QStringLiteral(", ")).toLocal8Bit().data());
        fprintf(out, "        },\n");
    }
}

static void generateTranslations(FILE *out, const QVector<ActionObjectInfoMessage> &objects) {
    fprintf(out, "    // Action Text\n");
    QSet<QString> texts{{}};
    for (const auto &item : std::as_const(objects)) {
        if (item.text.isEmpty())
            continue;
        texts.insert(item.commandClass);
        fprintf(out, "    QCoreApplication::translate(\"ChorusKit::ActionText\", \"%s\");\n",
                escapeString(item.text.toLocal8Bit()).data());
    }
    fprintf(out, "\n");

    fprintf(out, "    // Action Class\n");
    QSet<QString> commandClasses{{}};
    for (const auto &item : std::as_const(objects)) {
        if (commandClasses.contains(item.commandClass))
            continue;
        commandClasses.insert(item.commandClass);
        fprintf(out, "    QCoreApplication::translate(\"ChorusKit::ActionClass\", \"%s\");\n",
                escapeString(item.commandClass.toLocal8Bit()).data());
    }
    fprintf(out, "\n");

    fprintf(out, "    // Action Category\n");
    QSet<QString> categories{{}};
    for (const auto &item : std::as_const(objects)) {
        for (const auto &subItem : std::as_const(item.categories)) {
            if (categories.contains(subItem))
                continue;
            categories.insert(subItem);
            fprintf(out,
                    "    QCoreApplication::translate(\"ChorusKit::ActionCategory\", \"%s\");\n",
                    escapeString(subItem.toLocal8Bit()).data());
        }
    }
}

static void generateExtraInformation(FILE *out, const QVector<ActionObjectInfoMessage> &objects) {
    QVector<ActionObjectInfoMessage> actions;
    QVector<ActionObjectInfoMessage> widgets;
    QVector<ActionObjectInfoMessage> groups;
    QVector<ActionObjectInfoMessage> menus;
    QVector<ActionObjectInfoMessage> topLevels;

    for (const auto &item : objects) {
        if (item.typeToken == QStringLiteral("Action")) {
            if (item.modeToken == QStringLiteral("Plain")) {
                actions.append(item);
            } else {
                widgets.append(item);
            }
        } else if (item.typeToken == QStringLiteral("Group")) {
            groups.append(item);
        } else if (item.typeToken == QStringLiteral("Menu")) {
            if (item.modeToken == QStringLiteral("Plain")) {
                menus.append(item);
            } else {
                topLevels.append(item);
            }
        }
    }

    fprintf(out, "/*\n");

    // // Actions
    // fprintf(out, "    [Action]\n");
    // for (const auto &item : std::as_const(actions)) {
    //     fprintf(out, "    %s\n", item.id.toLocal8Bit().data());
    // }
    // fprintf(out, "\n");

    // // Widgets
    // fprintf(out, "    [Widget]\n");
    // for (const auto &item : std::as_const(widgets)) {
    //     fprintf(out, "    %s\n", item.id.toLocal8Bit().data());
    // }
    // fprintf(out, "\n");

    // // Groups
    // fprintf(out, "    [Group]\n");
    // for (const auto &item : std::as_const(groups)) {
    //     if (item.typeToken == QStringLiteral("Group")) {
    //         fprintf(out, "    %s\n", item.id.toLocal8Bit().data());
    //     }
    // }
    // fprintf(out, "\n");

    // // Menus
    // fprintf(out, "    [Menu]\n");
    // for (const auto &item : std::as_const(menus)) {
    //     fprintf(out, "    %s\n", item.id.toLocal8Bit().data());
    // }
    // fprintf(out, "\n");

    // // TopLevels
    // fprintf(out, "    [TopLevel]\n");
    // for (const auto &item : std::as_const(topLevels)) {
    //     fprintf(out, "    %s\n", item.id.toLocal8Bit().data());
    // }
    // fprintf(out, "\n");

    // Hint
    fprintf(out, "    You can define a struct representing the corresponding instances "
                 "with this extension:\n\n");

    if (!actions.isEmpty()) {
        fprintf(out, "    struct ActionItems {\n");
        for (const auto &item : std::as_const(actions)) {
            fprintf(out, "        Core::ActionItem *%s;\n", item.id.toLocal8Bit().data());
        }
        fprintf(out, "    };\n");
        fprintf(out, "\n");
    }

    if (!widgets.isEmpty()) {
        fprintf(out, "    struct WidgetItems {\n");
        for (const auto &item : std::as_const(widgets)) {
            fprintf(out, "        Core::ActionItem *%s;\n", item.id.toLocal8Bit().data());
        }
        fprintf(out, "    };\n");
        fprintf(out, "\n");
    }

    if (!groups.isEmpty()) {
        fprintf(out, "    struct GroupItemes {\n");
        for (const auto &item : std::as_const(groups)) {
            fprintf(out, "        Core::ActionItem *%s;\n", item.id.toLocal8Bit().data());
        }
        fprintf(out, "    };\n");
        fprintf(out, "\n");
    }

    if (!menus.isEmpty()) {
        fprintf(out, "    struct MenuItems {\n");
        for (const auto &item : std::as_const(menus)) {
            fprintf(out, "        Core::ActionItem *%s;\n", item.id.toLocal8Bit().data());
        }
        fprintf(out, "    };\n");
        fprintf(out, "\n");
    }

    if (!topLevels.isEmpty()) {
        fprintf(out, "    struct TopLevelItems {\n");
        for (const auto &item : std::as_const(topLevels)) {
            fprintf(out, "        Core::ActionItem *%s;\n", item.id.toLocal8Bit().data());
        }
        fprintf(out, "    };\n");
    }

    fprintf(out, "*/\n");
}

void Generator::generateCode() {
    // Warning
    fprintf(out,
            "/****************************************************************************\n"
            "** Action extension structure code from reading XML file '%s'\n**\n",
            inputFileName.constData());
    fprintf(out, "** Created by: ChorusKit Action Extension Compiler version %s (Qt %s)\n**\n",
            APP_VERSION, QT_VERSION_STR);
    fprintf(out, "** WARNING! All changes made in this file will be lost!\n"
                 "*************************************************************************"
                 "****/\n");

    fprintf(out, R"(
#include <QtCore/QCoreApplication>

#include <CoreApi/private/actionextension_p.h>

)");

    fprintf(out, "namespace ckStaticActionExtension_%s {\n", identifier.data());

    fprintf(out, R"(
using namespace Core;

static ActionExtensionPrivate *get_data() {
    static ActionExtensionPrivate data;
)");

    fprintf(out, "    data.hash = QStringLiteral(\"%s\");\n", msg.hash.toLocal8Bit().data());
    fprintf(out, "    data.version = QStringLiteral(\"%s\");\n",
            escapeString(msg.version.toLocal8Bit()).data());
    fprintf(out, "\n");

    if (msg.objects.isEmpty()) {
        fprintf(out, "    data.objectData = nullptr;\n");
        fprintf(out, "    data.objectCount = 0;\n");
    } else {
        fprintf(out, "    static ActionObjectInfoData objectData[] = {\n");
        generateObjects(out, msg.objects);
        fprintf(out, "    };\n");
        fprintf(out, "    data.objectData = objectData;\n");
        fprintf(out, "    data.objectCount = sizeof(objectData) / sizeof(objectData[0]);\n");
    }
    fprintf(out, "\n");

    if (msg.layouts.isEmpty()) {
        fprintf(out, "    data.layoutEntryData = nullptr;\n");
        fprintf(out, "    data.layoutEntryCount = 0;\n");
    } else {
        fprintf(out, "    static ActionLayoutInfoEntry layoutEntryData[] = {\n");
        generateLayouts(out, msg.layouts);
        fprintf(out, "    };\n");
        fprintf(out, "    data.layoutEntryData = layoutEntryData;\n");
        fprintf(
            out,
            "    data.layoutEntryCount = sizeof(layoutEntryData) / sizeof(layoutEntryData[0]);\n");
    }
    fprintf(out, "\n");

    if (msg.layoutRootIndexes.isEmpty()) {
        fprintf(out, "    data.layoutRootData = nullptr;\n");
        fprintf(out, "    data.layoutRootCount = 0;\n");
    } else {
        fprintf(out, "    static int layoutRootData[] = {\n");
        fprintf(out, "        %s\n",
                joinNumbers(msg.layoutRootIndexes, QStringLiteral(", ")).toLocal8Bit().data());
        fprintf(out, "    };\n");
        fprintf(out, "    data.layoutRootData = layoutRootData;\n");
        fprintf(out,
                "    data.layoutRootCount = sizeof(layoutRootData) / sizeof(layoutRootData[0]);\n");
    }
    fprintf(out, "\n");

    if (msg.buildRoutines.isEmpty()) {
        fprintf(out, "    data.buildRoutineData = nullptr;\n");
        fprintf(out, "    data.buildRoutineCount = 0;\n");
    } else {
        fprintf(out, "    static ActionBuildRoutineData buildRoutineData[] = {\n");
        generateBuildRoutines(out, msg.buildRoutines);
        fprintf(out, "    };\n");
        fprintf(out, "    data.buildRoutineData = buildRoutineData;\n");
        fprintf(out, "    data.buildRoutineCount = sizeof(buildRoutineData) / "
                     "sizeof(buildRoutineData[0]);\n");
    }
    fprintf(out, "\n");

    fprintf(out, R"(    return &data;
}

}

)");

    fprintf(out,
            "const Core::ActionExtension *QT_MANGLE_NAMESPACE(ckGetStaticActionExtension_%s)() {\n",
            identifier.data());
    fprintf(out, "    static Core::ActionExtension extension{\n");
    fprintf(out, "        {\n");
    fprintf(out, "            ckStaticActionExtension_%s::get_data(),\n", identifier.data());
    fprintf(out, "        },\n");
    fprintf(out, "    };\n");

    fprintf(out, R"(    return &extension;
}

#if 0
// This field is only used to generate translation files for the Qt linguist tool
)");

    fprintf(out, "static void ckDeclareStaticActionTranslations_%s() {\n", identifier.data());
    generateTranslations(out, msg.objects);
    fprintf(out, R"(}
#endif
)");
    fprintf(out, "\n");

    // Extra information
    generateExtraInformation(out, msg.objects);
}
