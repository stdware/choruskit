#include "generator.h"

#include <QSet>

void error(const char *msg);

Generator::Generator(FILE *out, const QByteArray &inputFileName, const QByteArray &identifier,
                     const ActionExtensionMessage &message)
    : out(out), inputFileName(inputFileName), identifier(identifier), msg(message) {
}

static void generateObjects(FILE *out, const QVector<ActionObjectInfoMessage> &objects) {
    for (const auto &item : std::as_const(objects)) {
        fprintf(out, "        {\n");
        fprintf(out, "            // id\n");
        fprintf(out, "            QStringLiteral(\"%s\"),\n", item.id.toLatin1().data());
        fprintf(out, "            // type\n");
        fprintf(out, "            ActionObjectInfo::%s,\n", item.typeToken.toLatin1().data());
        fprintf(out, "            // text\n");
        fprintf(out, "            QByteArrayLiteral(\"%s\"),\n", item.text.toLatin1().data());
        fprintf(out, "            // commandClass\n");
        if (item.commandClass.isEmpty()) {
            fprintf(out, "            QByteArray(),\n");
        } else {
            fprintf(out, "            QByteArrayLiteral(\"%s\"),\n",
                    item.commandClass.toLatin1().data());
        }
        fprintf(out, "            // shortcuts\n");
        fprintf(out, "            {\n");
        for (const auto &subItem : std::as_const(item.shortcutTokens)) {
            fprintf(out, "                QKeySequence(\"%s\"),\n",
                    subItem.trimmed().toLatin1().data());
        }
        fprintf(out, "            },\n");
        fprintf(out, "            // categories\n");
        fprintf(out, "            {\n");
        for (const auto &subItem : std::as_const(item.categories)) {
            fprintf(out, "                QByteArrayLiteral(\"%s\"),\n", subItem.toLatin1().data());
        }
        fprintf(out, "            },\n");
        fprintf(out, "            // topLevel\n");
        fprintf(out, "            %s,\n", item.topLevel ? "true" : "false");
        fprintf(out, "        },\n");
    }
}

static void generateLayouts(FILE *out, const QVector<ActionLayoutMessage> &layouts) {
    for (const auto &item : std::as_const(layouts)) {
        fprintf(out, "        {{\n");
        int i = 0;
        for (const auto &subItem : std::as_const(item.entryData)) {
            fprintf(out, "            {\n");
            fprintf(out, "                // index %d\n", i++);
            fprintf(out, "                // id\n");
            if (subItem.id.isEmpty()) {
                fprintf(out, "                QString(),\n");
            } else {
                fprintf(out, "                QStringLiteral(\"%s\"),\n",
                        subItem.id.toLatin1().data());
            }
            fprintf(out, "                // type\n");
            fprintf(out, "                ActionObjectInfo::%s,\n",
                    subItem.typeToken.toLatin1().data());
            fprintf(out, "                // flat\n");
            fprintf(out, "                %s,\n", subItem.flat ? "true" : "false");
            fprintf(out, "                // childIndexes\n");
            fprintf(out, "                {%s},\n",
                    [](const decltype(subItem.childIndexes) &arr) {
                        QStringList res;
                        res.reserve(arr.size());
                        for (const auto &item : arr) {
                            res.append(QString::number(item));
                        }
                        return res;
                    }(subItem.childIndexes)
                        .join(QStringLiteral(", "))
                        .toLatin1()
                        .data());
            fprintf(out, "            },\n");
        }
        fprintf(out, "        }},\n");
    }
}

static void generateBuildRoutines(FILE *out, const QVector<ActionBuildRoutineMessage> &routines) {
    for (const auto &item : std::as_const(routines)) {
        fprintf(out, "        {\n");
        fprintf(out, "            // anchorToken\n");
        fprintf(out, "            ActionBuildRoutine::%s,\n", item.anchorToken.toLatin1().data());
        fprintf(out, "            // parent\n");
        fprintf(out, "            QStringLiteral(\"%s\"),\n", item.parent.toLatin1().data());
        fprintf(out, "            // relativeTo\n");
        if (item.relativeTo.isEmpty()) {
            fprintf(out, "            QString(),\n");
        } else {
            fprintf(out, "            QStringLiteral(\"%s\"),\n",
                    item.relativeTo.toLatin1().data());
        }

        fprintf(out, "            // items\n");
        fprintf(out, "            {\n");
        for (const auto &subItem : std::as_const(item.items)) {
            fprintf(out, "                {\n");
            fprintf(out, "                    // id\n");
            if (subItem.id.isEmpty()) {
                fprintf(out, "                    QString(),\n");
            } else {
                fprintf(out, "                    QStringLiteral(\"%s\"),\n",
                        subItem.id.toLatin1().data());
            }
            fprintf(out, "                    // type\n");
            fprintf(out, "                    ActionObjectInfo::%s,\n",
                    subItem.typeToken.toLatin1().data());
            fprintf(out, "                    // flat\n");
            fprintf(out, "                    %s,\n", subItem.flat ? "true" : "false");
            fprintf(out, "                },\n");
        }
        fprintf(out, "            },\n");
        fprintf(out, "        },\n");
    }
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

static ActionExtensionPrivate *getData() {
    static ActionExtensionPrivate data;
)");

    fprintf(out, "    data.hash = QStringLiteral(\"%s\");\n", msg.hash.toLatin1().data());
    fprintf(out, "    data.version = QStringLiteral(\"%s\");\n", msg.version.toLatin1().data());
    fprintf(out, "\n");

    if (msg.objects.isEmpty()) {
        fprintf(out, "    data.objectData = nullptr;\n");
        fprintf(out, "    data.objectCount = 0;\n");
    } else {
        fprintf(out, "    static ActionObjectInfoData objectData[] = {\n");
        generateObjects(out, msg.objects);
        fprintf(out, "    };\n");
        fprintf(out, "    data.objectData = objectData;\n");
        fprintf(out, "    data.objectCount = sizeof(objectData) / sizeof(ActionObjectInfoData);\n");
    }
    fprintf(out, "\n");

    if (msg.objects.isEmpty()) {
        fprintf(out, "    data.layoutData = nullptr;\n");
        fprintf(out, "    data.layoutCount = 0;\n");
    } else {
        fprintf(out, "    static ActionLayoutData layoutData[] = {\n");
        generateLayouts(out, msg.layouts);
        fprintf(out, "    };\n");
        fprintf(out, "    data.layoutData = layoutData;\n");
        fprintf(out, "    data.layoutCount = sizeof(layoutData) / sizeof(ActionLayoutData);\n");
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
                     "sizeof(ActionBuildRoutineData);\n");
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
    fprintf(out, "            ckStaticActionExtension_%s::getData(),\n", identifier.data());
    fprintf(out, "        },\n");
    fprintf(out, "    };\n");

    fprintf(out, R"(    return &extension;
}

#if 0
// This field is only used to generate translation files for the Qt linguist tool
)");

    fprintf(out, "static void ckDeclareStaticActionTranslations_%s() {\n", identifier.data());

    fprintf(out, "    // ActionText\n");
    QSet<QString> texts{{}};
    for (const auto &item : std::as_const(msg.objects)) {
        if (item.text.isEmpty())
            continue;
        texts.insert(item.commandClass);
        fprintf(out, "    QCoreApplication::translate(\"ChorusKit::ActionText\", \"%s\");\n",
                item.text.toLatin1().data());
    }
    fprintf(out, "\n");

    fprintf(out, "    // ActionClass\n");
    QSet<QString> commandClasses{{}};
    for (const auto &item : std::as_const(msg.objects)) {
        if (commandClasses.contains(item.commandClass))
            continue;
        commandClasses.insert(item.commandClass);
        fprintf(out, "    QCoreApplication::translate(\"ChorusKit::ActionClass\", \"%s\");\n",
                item.commandClass.toLatin1().data());
    }
    fprintf(out, "\n");

    fprintf(out, "    // ActionCategory\n");
    QSet<QString> categories{{}};
    for (const auto &item : std::as_const(msg.objects)) {
        for (const auto &subItem : std::as_const(item.categories)) {
            if (categories.contains(subItem))
                continue;
            categories.insert(subItem);
            fprintf(out,
                    "    QCoreApplication::translate(\"ChorusKit::ActionCategory\", \"%s\");\n",
                    subItem.toLatin1().data());
        }
    }
    fprintf(out, "\n");
    fprintf(out, R"(}
#endif
)");
}
