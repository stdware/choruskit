#include "parser.h"

#include <list>
#include <algorithm>

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QRegularExpression>

#include <qmxmladaptor.h>

void error(const char *msg);

static QString calculateContentSha256(const QByteArray &data) {
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(data);
    return hash.result().toHex();
}

static QString parseExpression(QString s, const QHash<QString, QString> &vars) {
    QRegularExpressionMatch match;
    int index = 0;
    bool hasMatch = false;

    static QRegularExpression reg(QStringLiteral(R"(\$\{(\w+)\})"));
    while ((index = s.indexOf(reg, index, &match)) != -1) {
        hasMatch = true;

        const auto &name = match.captured(1);
        QString val;
        auto it = vars.find(name);
        if (it == vars.end()) {
            val = name;
        } else {
            val = it.value();
        }

        s.replace(index, match.captured(0).size(), val);
    }
    if (!hasMatch) {
        return s;
    }
    return parseExpression(s, vars);
}

static QString objIdToText(const QString &id) {
    QStringList parts;
    QString currentPart;

    for (const auto &ch : id) {
        if (ch.isUpper() && !currentPart.isEmpty()) {
            parts.append(currentPart);
            currentPart.clear();
        }
        currentPart += ch;
    }
    if (!currentPart.isEmpty()) {
        parts.append(currentPart);
    }
    return parts.join(QChar(' '));
}

static QString removeAccelerateKeys(const QString &s) {
    QString res;

    for (int i = 0; i < s.size(); ++i) {
        const QChar &ch = s[i];
        if (ch == '&') {
            if (i + 1 < s.size()) {
                i++;
                res += s[i];
            }
            continue;
        }
        res += ch;
    }
    return res;
}

static QStringList parseStringList(const QString &s) {
    QStringList parts;
    QString currentPart;

    for (int i = 0; i < s.size(); ++i) {
        const QChar &ch = s[i];
        if (ch == '\\') {
            if (i + 1 < s.size()) {
                i++;
                currentPart += s[i];
            }
            continue;
        }
        if (ch == ';') {
            parts.append(currentPart);
            currentPart.clear();
            continue;
        }
        currentPart += ch;
    }
    parts.append(currentPart);
    return parts;
}

struct ParserPrivate {
    struct ParserConfig {
        QStringList defaultCategory;
    };

    QString fileName;
    QHash<QString, QString> variables;

    ActionExtensionMessage parse(const QByteArray &data) {
        QMXmlAdaptor xml;

        // Read file
        if (!xml.loadData(data)) {
            fprintf(stderr, "%s:%s: invalid format\n", qPrintable(qApp->applicationName()),
                    qPrintable(fileName));
            std::exit(1);
        }

        // Check root name
        const auto &root = xml.root;
        if (const auto &rootname = root.name; rootname != QStringLiteral("extension")) {
            fprintf(stderr, "%s:%s: unknown root element tag %s\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    rootname.toLatin1().data());
            std::exit(1);
        }

        QList<QMXmlAdaptorElement *> objElements;
        QList<QMXmlAdaptorElement *> layoutElements;
        QList<QMXmlAdaptorElement *> routineElements;

        // Parse root elements and collect attributes
        QString version;
        bool hasParserConfig = false;
        ParserConfig parserConfig;
        for (const auto &item : std::as_const(root.children)) {
            if (item->name == QStringLiteral("objects")) {
                for (const auto &subItem : std::as_const(item->children)) {
                    objElements.append(subItem.data());
                }
                continue;
            }
            if (item->name == QStringLiteral("layouts")) {
                for (const auto &subItem : std::as_const(item->children)) {
                    layoutElements.append(subItem.data());
                }
                continue;
            }
            if (item->name == QStringLiteral("buildRoutines")) {
                for (const auto &subItem : std::as_const(item->children)) {
                    routineElements.append(subItem.data());
                }
                continue;
            }
            if (item->name == QStringLiteral("version")) {
                if (!version.isEmpty()) {
                    fprintf(stderr,
                            "%s:%s: duplicated version value \"%s\", the previous one is \"%s\"\n",
                            qPrintable(qApp->applicationName()), qPrintable(fileName),
                            item->value.toLatin1().data(), version.toLatin1().data());
                    std::exit(1);
                }
                version = item->value;
                continue;
            }
            if (item->name == QStringLiteral("parserConfig")) {
                if (hasParserConfig) {
                    fprintf(stderr, "%s:%s: duplicated parser config elements\n",
                            qPrintable(qApp->applicationName()), qPrintable(fileName));
                    std::exit(1);
                }
                parserConfig = parseParserConfig(*item);
                hasParserConfig = true;
                continue;
            }
        }

        // Build result
        ActionExtensionMessage result;
        result.version = version;
        result.hash = calculateContentSha256(data);

        // Parse objects
        QHash<QString, ActionObjectInfoMessage> objMap;
        QStringList objIdSeq;
        for (const auto &item : std::as_const(objElements)) {
            auto entity = parseObject(*item);
            if (objMap.contains(entity.id)) {
                fprintf(stderr, "%s:%s: duplicated object id %s\n",
                        qPrintable(qApp->applicationName()), qPrintable(fileName),
                        entity.id.toLatin1().data());
                std::exit(1);
            }
            objMap.insert(entity.id, entity);
            objIdSeq.append(entity.id);
        }

        // Parse layouts
        QSet<QString> organizedObjects;
        for (const auto &item : std::as_const(layoutElements)) {
            auto entity = parseLayout(*item, parserConfig.defaultCategory, objMap, objIdSeq,
                                      organizedObjects);
            result.layouts.append(entity);
        }

        // Parse build routines
        for (const auto &item : std::as_const(routineElements)) {
            auto entity = parseRoutine(*item, parserConfig.defaultCategory, objMap, objIdSeq);
            result.buildRoutines.append(entity);
        }

        // Collect objects
        for (const auto &id : std::as_const(objIdSeq)) {
            result.objects.append(objMap.value(id));
        }

        return result;
    }

    ParserConfig parseParserConfig(const QMXmlAdaptorElement &e) const {
        ParserConfig result;
        for (const auto &item : e.children) {
            if (item->name == QStringLiteral("defaultCategory")) {
                result.defaultCategory = parseStringList(item->value);
                continue;
            }
        }
        return result;
    }

    void determineElementActionType(const QMXmlAdaptorElement &e, ActionObjectInfoMessage &result,
                                    const char *field) const {
        auto name = e.name;
        if (name == QStringLiteral("action")) {
            result.typeToken = QStringLiteral("Action");
        } else if (name == QStringLiteral("group")) {
            result.typeToken = QStringLiteral("Group");
        } else if (name == QStringLiteral("menuBar") || name == QStringLiteral("toolBar")) {
            result.typeToken = QStringLiteral("Menu");
            result.topLevel = true;
        } else if (name == QStringLiteral("menu")) {
            result.typeToken = QStringLiteral("Menu");
        } else {
            fprintf(stderr, "%s:%s: unknown %s object tag \"%s\"\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName), field,
                    name.toLatin1().data());
            std::exit(1);
        }
        result.tag = e.name;
    }

    ActionObjectInfoMessage parseObject(const QMXmlAdaptorElement &e) {
        ActionObjectInfoMessage result;
        if (auto id = e.properties.value(QStringLiteral("id")); id.isEmpty()) {
            fprintf(stderr, "%s:%s: object element \"%s\" doesn't have an \"id\" field\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    e.name.toLatin1().data());
            std::exit(1);
        } else {
            result.id = id;
        }

        // type
        result.topLevel = e.properties.value("top") == QStringLiteral("true");
        determineElementActionType(e, result, "object");

        // text
        if (auto text = e.properties.value(QStringLiteral("text")); text.isEmpty()) {
            result.text = objIdToText(result.id);
        }

        // class
        if (auto commandClass = e.properties.value(QStringLiteral("class"));
            !commandClass.isEmpty()) {
            result.commandClass = commandClass;
        }

        // shortcuts
        if (auto shortcuts = e.properties.value(QStringLiteral("shortcuts"));
            !shortcuts.isEmpty()) {
            result.shortcutTokens = parseStringList(shortcuts);
        } else if (shortcuts = e.properties.value(QStringLiteral("shortcut"));
                   !shortcuts.isEmpty()) {
            result.shortcutTokens = parseStringList(shortcuts);
        }

        // categories
        if (auto categories = e.properties.value(QStringLiteral("categories"));
            !categories.isEmpty()) {
            result.categories = parseStringList(categories);
        } else if (categories = e.properties.value(QStringLiteral("category"));
                   !categories.isEmpty()) {
            result.categories = parseStringList(categories);
        }

        return result;
    }

    ActionLayoutMessage parseLayout(const QMXmlAdaptorElement &root,
                                    const QStringList &defaultCategory,
                                    QHash<QString, ActionObjectInfoMessage> &objMap,
                                    QStringList &objIdSeq, QSet<QString> &organizedObjects) {
        ActionLayoutMessage result;
        auto &entries = result.entryData;

        struct Element {
            QStringList category;
            const QMXmlAdaptorElement *e;
            int entryIndex;
        };
        std::list<Element> stack;
        result.entryData.append(ActionLayoutMessage::Entry{});
        stack.push_back({defaultCategory, &root, 0});
        while (!stack.empty()) {
            auto top = stack.back();
            stack.pop_back();

            const auto &e = *top.e;
            auto id = e.properties.value(QStringLiteral("id"));
            if (id.isEmpty()) {
                fprintf(stderr, "%s:%s: layout element \"%s\" doesn't have an \"id\" field\n",
                        qPrintable(qApp->applicationName()), qPrintable(fileName),
                        e.name.toLatin1().data());
                std::exit(1);
            }

            // Calculate current category
            QStringList currentCategory = top.category;
            QString typeToken;
            {
                auto it = objMap.find(id);
                if (it != objMap.end()) {
                    // This layout object has been declared in the objects field
                    auto &info = it.value();

                    // Check if the tag matches
                    if (info.tag != e.name) {
                        fprintf(
                            stderr,
                            "%s:%s: layout element of \"%s\" has inconsistent tag \"%s\" with the "
                            "object element \"%s\"\n",
                            qPrintable(qApp->applicationName()), qPrintable(fileName),
                            id.toLatin1().data(), e.name.toLatin1().data(),
                            info.tag.toLatin1().data());
                        std::exit(1);
                    }

                    if (info.categories.isEmpty()) {
                        // The object doesn't have a specified category, use the current one
                        info.categories = top.category;
                    } else {
                        currentCategory = info.categories;
                    }
                    currentCategory += removeAccelerateKeys(info.text);
                    typeToken = info.typeToken;
                } else {
                    // Create one
                    ActionObjectInfoMessage info;
                    info.id = id;
                    determineElementActionType(e, info, "layout");
                    info.text = objIdToText(id);
                    info.categories = top.category;
                    objMap.insert(id, info);
                    objIdSeq.append(id);
                    currentCategory += info.text;
                    typeToken = info.typeToken;
                }
            }

            ActionLayoutMessage::Entry &entry = entries[top.entryIndex];
            entry.id = id;
            entry.typeToken = typeToken;
            entry.flat = e.properties.value(QStringLiteral("flat")) == QStringLiteral("true");
            if (e.children.isEmpty() && organizedObjects.contains(id)) {
                fprintf(stderr, "%s:%s: layout element \"%s\" has multiple defined structures\n",
                        qPrintable(qApp->applicationName()), qPrintable(fileName),
                        id.toLatin1().data());
                std::exit(1);
            }
            organizedObjects.insert(id);

            for (const auto &item : e.children) {
                ActionLayoutMessage::Entry childEntry;
                int currentEntryIndex = entries.size();
                if (item->name == QStringLiteral("separator")) {
                    childEntry.typeToken = QStringLiteral("Separator");
                } else if (item->name == QStringLiteral("stretch")) {
                    childEntry.typeToken = QStringLiteral("Stretch");
                } else {
                    // Deferred resolve
                    stack.push_back({currentCategory, item.data(), currentEntryIndex});
                }
                entry.childIndexes.append(currentEntryIndex);
                entries.append(childEntry);
            }
        }
        return result;
    }

    ActionBuildRoutineMessage parseRoutine(const QMXmlAdaptorElement &root,
                                           const QStringList &defaultCategory,
                                           QHash<QString, ActionObjectInfoMessage> &objMap,
                                           QStringList &objIdSeq) {
        if (const auto &rootname = root.name; rootname != QStringLiteral("buildRoutine")) {
            fprintf(stderr, "%s:%s: unknown build routine element tag %s\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    rootname.toLatin1().data());
            std::exit(1);
        }

        auto parent = root.properties.value(QStringLiteral("parent"));
        if (parent.isEmpty()) {
            fprintf(stderr, "%s:%s: build routine doesn't have a parent\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName));
            std::exit(1);
        }

        auto anchor = root.properties.value(QStringLiteral("anchor"));
        QString anchorToken;
        bool needRelative = false;
        if (anchor == "last" || anchor == "back") {
            anchorToken = "Last";
        } else if (anchor == "first" || anchor == "front") {
            anchorToken = "First";
        } else if (anchor == "before") {
            anchorToken = "Before";
            needRelative = true;
        } else if (anchor.isEmpty() || anchor == "after") {
            anchorToken = "After";
            needRelative = true;
        } else {
            fprintf(stderr, "%s:%s: unknown build routine anchor %s\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    anchor.toLatin1().data());
            std::exit(1);
        }

        auto relative = root.properties.value(QStringLiteral("relativeTo"));
        if (needRelative && relative.isEmpty()) {
            fprintf(stderr,
                    "%s:%s: build routine with anchor \"%s\" must have a relative sibling\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    anchor.toLatin1().data());
            std::exit(1);
        }

        ActionBuildRoutineMessage result;
        result.anchorToken = anchorToken;
        result.parent = parent;
        result.relativeTo = relative;

        if (root.children.isEmpty()) {
            fprintf(stderr, "%s:%s: empty build routine\n", qPrintable(qApp->applicationName()),
                    qPrintable(fileName));
            std::exit(1);
        }

        for (const auto &item : root.children) {
        }
        return result;
    }
};

Parser::Parser() = default;

ActionExtensionMessage Parser::parse(const QByteArray &data) const {
    return ParserPrivate{fileName, variables}.parse(data);
}
