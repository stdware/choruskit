#include "parser.h"

#include <algorithm>

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QRegularExpression>
#include <utility>

#include <QMCore/qmchronomap.h>

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

    // Lower case special words
    static QSet<QString> specialWords{
        QStringLiteral("and"), QStringLiteral("but"),  QStringLiteral("or"), QStringLiteral("nor"),
        QStringLiteral("for"), QStringLiteral("yet"),  QStringLiteral("so"), QStringLiteral("as"),
        QStringLiteral("at"),  QStringLiteral("by"),   QStringLiteral("in"), QStringLiteral("of"),
        QStringLiteral("on"),  QStringLiteral("to"),   QStringLiteral("up"), QStringLiteral("a"),
        QStringLiteral("an"),  QStringLiteral("the "),
    };
    for (auto &part : parts) {
        if (auto lower = part.toLower(); specialWords.contains(lower)) {
            part = lower;
        }
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

static inline bool isStringDigits(const QString &s) {
    return std::all_of(s.begin(), s.end(), [](const QChar &ch) { return ch.isDigit(); });
}

static void fixCategories(ActionObjectInfoMessage &info) {
    QStringList res;
    for (auto it = info.categories.begin(); it != info.categories.end() - 1; ++it) {
        if (it->isEmpty())
            continue;
        res.append(*it);
    }
    if (info.categories.back().isEmpty()) {
        res += removeAccelerateKeys(info.text);
    }
    info.categories = res;
}

struct ParserPrivate {
    struct ParserConfig {
        QStringList defaultCategory;
    };

    QString fileName;
    QHash<QString, QString> variables;

    ParserConfig parserConfig;
    QMChronoMap<QString, ActionObjectInfoMessage> objInfoMap;
    QHash<QString, QMChronoMap<QString, int>> objSeqMap; // id -> [seq -> index]
    ActionExtensionMessage result;

    ParserPrivate(QString fileName, const QHash<QString, QString> &variables)
        : fileName(std::move(fileName)), variables(variables) {
    }

    inline QString resolve(const QString &s) const {
        return parseExpression(s, variables);
    }

    ActionObjectInfoMessage &findOrInsertObjectInfo(const QMXmlAdaptorElement *e,
                                                    const QStringList &categories,
                                                    const char *field) {
        auto id = resolve(e->properties.value(QStringLiteral("id")));
        if (id.isEmpty()) {
            fprintf(stderr, "%s:%s: %s element \"%s\" doesn't have an \"id\" field\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName), field,
                    e->name.toLatin1().data());
            std::exit(1);
        }

        ActionObjectInfoMessage *pInfo;
        if (auto it = objInfoMap.find(id); it != objInfoMap.end()) {
            // This layout object has been declared in the objects field
            auto &info = it.value();

            // Check if the tag matches
            if (info.tag != e->name && info.tag != QStringLiteral("object")) {
                fprintf(stderr,
                        "%s:%s: %s element \"%s\" has inconsistent tag \"%s\" with the "
                        "object element \"%s\"\n",
                        qPrintable(qApp->applicationName()), qPrintable(fileName), field,
                        id.toLatin1().data(), e->name.toLatin1().data(),
                        info.tag.toLatin1().data());
                std::exit(1);
            }

            if (info.categories.isEmpty()) {
                // The object doesn't have a specified category, use the current one
                info.categories = QStringList(categories) << removeAccelerateKeys(info.text);
            }
            pInfo = &info;
        } else {
            // Create one
            ActionObjectInfoMessage info;
            info.id = id;
            determineObjectType(*e, info, field);
            info.text = objIdToText(id);
            info.categories = QStringList(categories) << info.text;

            auto insertResult = objInfoMap.append(id, info);
            pInfo = &insertResult.first.value();
        }
        return *pInfo;
    }

    void parse(const QByteArray &data) {
        QMXmlAdaptor xml;

        // Read file
        if (!xml.loadData(data)) {
            fprintf(stderr, "%s:%s: invalid format\n", qPrintable(qApp->applicationName()),
                    qPrintable(fileName));
            std::exit(1);
        }

        // Check root name
        const auto &root = xml.root;
        if (const auto &rootName = root.name; rootName != QStringLiteral("actionExtension")) {
            fprintf(stderr, "%s:%s: unknown root element tag %s\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    rootName.toLatin1().data());
            std::exit(1);
        }

        QList<QMXmlAdaptorElement *> objElements;
        QList<QMXmlAdaptorElement *> layoutElements;
        QList<QMXmlAdaptorElement *> routineElements;

        // Collect elements and attributes
        QString version;
        bool hasParserConfig = false;
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
        result.version = version;
        result.hash = calculateContentSha256(data);

        // Parse objects
        for (const auto &item : std::as_const(objElements)) {
            auto entity = parseObject(*item);
            if (objInfoMap.contains(entity.id)) {
                fprintf(stderr, "%s:%s: duplicated object id %s\n",
                        qPrintable(qApp->applicationName()), qPrintable(fileName),
                        entity.id.toLatin1().data());
                std::exit(1);
            }
            objInfoMap.append(entity.id, entity);
        }

        // Parse layouts
        for (const auto &item : std::as_const(layoutElements)) {
            QStringList categories = parserConfig.defaultCategory;
            QStringList path;
            result.layoutRootIndexes.append(parseLayoutRecursively(item, categories, path));
        }

        // Parse build routines
        for (const auto &item : std::as_const(routineElements)) {
            auto entity = parseRoutine(*item);
            result.buildRoutines.append(entity);
        }

        // Collect objects
        for (const auto &item : std::as_const(objInfoMap)) {
            result.objects.append(item);
        }
    }

    ParserConfig parseParserConfig(const QMXmlAdaptorElement &e) {
        ParserConfig conf;

        for (const auto &item : e.children) {
            if (item->name == QStringLiteral("defaultCategory")) {
                conf.defaultCategory = parseStringList(resolve(item->value));
                continue;
            }

            if (item->name == QStringLiteral("vars")) {
                for (const auto &subItem : item->children) {
                    auto key = resolve(subItem->properties.value(QStringLiteral("key")));
                    auto value = resolve(subItem->properties.value(QStringLiteral("value")));
                    if (!key.isEmpty()) {
                        variables.insert(key, value);
                    }
                }
            }
        }
        return conf;
    }

    void determineObjectType(const QMXmlAdaptorElement &e, ActionObjectInfoMessage &info,
                             const char *field) const {
        const auto &name = e.name;
        info.shapeToken = QStringLiteral("Plain");
        if (name == QStringLiteral("action")) {
            info.typeToken = QStringLiteral("Action");
        } else if (name == QStringLiteral("widget")) {
            info.typeToken = QStringLiteral("Action");
            info.shapeToken = QStringLiteral("Widget");
        } else if (name == QStringLiteral("group")) {
            info.typeToken = QStringLiteral("Group");
        } else if (name == QStringLiteral("menuBar") || name == QStringLiteral("toolBar")) {
            info.typeToken = QStringLiteral("Menu");
            info.shapeToken = QStringLiteral("TopLevel");
        } else if (name == QStringLiteral("menu")) {
            info.typeToken = QStringLiteral("Menu");
            if (resolve(e.properties.value(QStringLiteral("shape"))) == QStringLiteral("topLevel"))
                info.shapeToken = QStringLiteral("TopLevel");
        } else {
            fprintf(stderr, "%s:%s: unknown %s object tag \"%s\"\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName), field,
                    name.toLatin1().data());
            std::exit(1);
        }
        info.tag = e.name;
    }

    ActionObjectInfoMessage parseObject(const QMXmlAdaptorElement &e) {
        ActionObjectInfoMessage info;
        auto id = resolve(e.properties.value(QStringLiteral("id")));
        if (id.isEmpty()) {
            fprintf(stderr, "%s:%s: object element \"%s\" doesn't have an \"id\" field\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    e.name.toLatin1().data());
            std::exit(1);
        }
        info.id = id;

        // type
        determineObjectType(e, info, "object");

        // text
        if (auto text = resolve(e.properties.value(QStringLiteral("text"))); !text.isEmpty()) {
            info.text = text;
        } else {
            info.text = objIdToText(info.id);
        }

        // class
        if (auto commandClass = resolve(e.properties.value(QStringLiteral("class")));
            !commandClass.isEmpty()) {
            info.commandClass = commandClass;
        }

        // shortcuts
        if (auto shortcuts = resolve(e.properties.value(QStringLiteral("shortcuts")));
            !shortcuts.isEmpty()) {
            info.shortcutTokens = parseStringList(shortcuts);
        } else if (shortcuts = resolve(e.properties.value(QStringLiteral("shortcut")));
                   !shortcuts.isEmpty()) {
            info.shortcutTokens = parseStringList(shortcuts);
        }

        // categories
        if (auto categories = resolve(e.properties.value(QStringLiteral("categories")));
            !categories.isEmpty()) {
            info.categories = parseStringList(categories);
            fixCategories(info);
        } else if (categories = resolve(e.properties.value(QStringLiteral("category")));
                   !categories.isEmpty()) {
            info.categories = parseStringList(categories);
            fixCategories(info);
        }

        if (!e.children.isEmpty()) {
            fprintf(stderr, "%s:%s: object declaration element \"%s\" shouldn't have children\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    e.name.toLatin1().data());
            std::exit(1);
        }

        return info;
    }

    int parseLayoutRecursively(const QMXmlAdaptorElement *e, QStringList &categories,
                               QStringList &path) {
        const auto &checkChildren = [this, e](const char *name) {
            if (!e->children.isEmpty()) {
                fprintf(stderr, "%s:%s: layout element %s shouldn't have children\n",
                        qPrintable(qApp->applicationName()), qPrintable(fileName), name);
                std::exit(1);
            }
        };

        auto &entries = result.layouts;
        ActionLayoutEntryMessage entry;
        int entryIndex = entries.size();
        if (e->name == QStringLiteral("separator")) {
            checkChildren("separator");
            entry.typeToken = QStringLiteral("Separator");
            entries.append(entry);
            return entryIndex;
        } else if (e->name == QStringLiteral("stretch")) {
            checkChildren("stretch");
            entry.typeToken = QStringLiteral("Stretch");
            entries.append(entry);
            return entryIndex;
        }

        auto &info = findOrInsertObjectInfo(e, categories, "layout");
        QString id = info.id;

        // Check recursive
        if (path.contains(id)) {
            fprintf(stderr, "%s:%s: recursive chain in layout: %s\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    (QStringList(path) << id).join(", ").toLatin1().data());
            std::exit(1);
        }
        entry.id = id;
        entry.typeToken = info.typeToken;

        if (info.typeToken == QStringLiteral("Action")) {
            entry.typeToken = QStringLiteral("Action");
            checkChildren(QString(R"("%1")").arg(id).toLatin1());
            entries.append(entry);
            return entryIndex;
        } else if (info.typeToken == QStringLiteral("Menu")) {
            if (resolve(e->properties.value(QStringLiteral("flat"))) == QStringLiteral("true")) {
                entry.typeToken = QStringLiteral("ExpandedMenu");
            } else {
                entry.typeToken = QStringLiteral("Menu");
            }
        } else {
            entry.typeToken = QStringLiteral("Group");
        }

        QString seq;
        auto &seqs = objSeqMap[id];

        {
            auto it = e->properties.find(QStringLiteral("_seq"));
            auto autoSeq = QString::number(seqs.size());
            if (it == e->properties.end()) {
                seq = (e->children.isEmpty() && !seqs.isEmpty()) ? seqs.begin().key() : autoSeq;
            } else {
                const auto &specifiedSeq = resolve(it.value());
                seq = (!seqs.contains(specifiedSeq) && isStringDigits(specifiedSeq)) ? autoSeq
                                                                                     : specifiedSeq;
            }
        }

        if (auto it = seqs.find(seq); it == seqs.end()) {
            seqs.append(seq, entryIndex);
            entries.append(entry); // Make a placeholder
        } else {
            if (info.shapeToken == QStringLiteral("TopLevel")) {
                fprintf(stderr,
                        "%s:%s: layout element \"%s\" has multiple defined structures while it's "
                        "top level\n",
                        qPrintable(qApp->applicationName()), qPrintable(fileName),
                        id.toLatin1().data());
                std::exit(1);
            }
            auto typeToken = entry.typeToken;
            entry = entries.at(it.value());
            entry.typeToken = typeToken;
            entries.append(entry);
            return entryIndex;
        }

        if (e->children.isEmpty()) {
            return entryIndex;
        }

        auto oldCategory = categories;
        categories = info.categories;
        path << id;

        QVector<int> childIndexes;
        childIndexes.reserve(e->children.size());
        for (const auto &child : e->children) {
            childIndexes.append(parseLayoutRecursively(child.data(), categories, path));
        }

        categories = oldCategory;
        path.removeLast();

        entries[entryIndex].childIndexes = childIndexes;
        return entryIndex;
    }

    ActionBuildRoutineMessage parseRoutine(const QMXmlAdaptorElement &root) {
        auto &entries = result.layouts;

        if (const auto &rootName = root.name; rootName != QStringLiteral("buildRoutine")) {
            fprintf(stderr, "%s:%s: unknown build routine element tag %s\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    rootName.toLatin1().data());
            std::exit(1);
        }

        auto parent = resolve(root.properties.value(QStringLiteral("parent")));
        if (parent.isEmpty()) {
            fprintf(stderr, "%s:%s: build routine doesn't have a parent\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName));
            std::exit(1);
        }

        auto anchor = root.properties.value(QStringLiteral("anchor"));
        QString anchorToken;
        bool needRelative = false;
        if (anchor == QStringLiteral("last") || anchor == QStringLiteral("back")) {
            anchorToken = QStringLiteral("Last");
        } else if (anchor == QStringLiteral("first") || anchor == QStringLiteral("front")) {
            anchorToken = QStringLiteral("First");
        } else if (anchor == QStringLiteral("before")) {
            anchorToken = QStringLiteral("Before");
            needRelative = true;
        } else if (anchor.isEmpty() || anchor == QStringLiteral("after")) {
            anchorToken = QStringLiteral("After");
            needRelative = true;
        } else {
            fprintf(stderr, "%s:%s: unknown build routine anchor %s\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    anchor.toLatin1().data());
            std::exit(1);
        }

        auto relative = resolve(root.properties.value(QStringLiteral("relativeTo")));
        if (needRelative && relative.isEmpty()) {
            fprintf(stderr,
                    "%s:%s: build routine with anchor \"%s\" must have a relative sibling\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    anchor.toLatin1().data());
            std::exit(1);
        }

        ActionBuildRoutineMessage routine;
        routine.anchorToken = anchorToken;
        routine.parent = parent;
        routine.relativeTo = relative;

        if (root.children.isEmpty()) {
            fprintf(stderr, "%s:%s: empty routine\n", qPrintable(qApp->applicationName()),
                    qPrintable(fileName));
            std::exit(1);
        }

        for (const auto &item : root.children) {
            auto &e = *item;
            ActionLayoutEntryMessage entry;
            int entryIndex = entries.size();
            if (e.name == QStringLiteral("separator")) {
                entry.typeToken = QStringLiteral("Separator");
            } else if (e.name == QStringLiteral("stretch")) {
                entry.typeToken = QStringLiteral("Stretch");
            } else {
                auto &info = findOrInsertObjectInfo(&e, parserConfig.defaultCategory, "routine");
                auto id = info.id;
                if (!e.children.isEmpty()) {
                    fprintf(stderr, "%s:%s: routine element \"%s\" shouldn't have children\n",
                            qPrintable(qApp->applicationName()), qPrintable(fileName),
                            e.name.toLatin1().data());
                    std::exit(1);
                }
                entry.id = id;
                entry.typeToken = info.typeToken;

                if (info.typeToken == QStringLiteral("Action")) {
                    entry.typeToken = QStringLiteral("Action");
                } else if (info.typeToken == QStringLiteral("Menu")) {
                    if (resolve(e.properties.value(QStringLiteral("flat"))) ==
                        QStringLiteral("true")) {
                        entry.typeToken = QStringLiteral("ExpandedMenu");
                    } else {
                        entry.typeToken = QStringLiteral("Menu");
                    }
                } else {
                    entry.typeToken = QStringLiteral("Group");
                }

                int idx = -1;
                auto seqs = objSeqMap.value(id);
                if (!seqs.isEmpty()) {
                    auto it = e.properties.find(QStringLiteral("_seq"));
                    if (it == e.properties.end()) {
                        idx = seqs.begin().value();
                    } else {
                        idx = seqs.value(resolve(it.value()), -1);
                    }
                }

                if (idx >= 0) {
                    auto typeToken = entry.typeToken;
                    entry = entries.at(idx);
                    entry.typeToken = typeToken;
                }
            }
            entries.append(entry);
            routine.entryIndexes.append(entryIndex);
        }
        return routine;
    }
};

Parser::Parser() = default;

ActionExtensionMessage Parser::parse(const QByteArray &data) const {
    ParserPrivate parser(fileName, variables);
    parser.parse(data);
    return parser.result;
}
