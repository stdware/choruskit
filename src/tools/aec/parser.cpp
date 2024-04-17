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

Parser::Parser() = default;

struct ParserConfig {
    QStringList defaultCategory;
};

static QString itemIdToText(const QString &id) {
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

static QByteArrayList stringListToLatin1(const QStringList &arr) {
    QByteArrayList res;
    res.reserve(arr.size());
    for (const auto &item : arr) {
        res.append(item.toLatin1());
    }
    return res;
}

static void determineItemType(const QString &fileName, const QMXmlAdaptorElement &e,
                              ActionItemInfoMessage &result) {
    auto name = e.name;
    if (name == "action") {
        result.typeToken = "Action";
    } else if (name == "group") {
        result.typeToken = "Group";
    } else if (name == "menuBar" || name == "toolBar") {
        result.typeToken = "Menu";
        result.topLevel = true;
    } else if (name == "menu") {
        result.typeToken = "Menu";
        result.topLevel = e.properties.value("top") == QStringLiteral("true");
    } else {
        fprintf(stderr, "%s:%s: unknown tag of action item \"%s\"\n",
                qPrintable(qApp->applicationName()), qPrintable(fileName), name.toLatin1().data());
        std::exit(1);
    }
    result.tag = e.name;
}


static ActionItemInfoMessage parseItem(const QString &fileName, const QMXmlAdaptorElement &e) {
    ActionItemInfoMessage result;
    if (auto id = e.properties.value(QStringLiteral("id")); id.isEmpty()) {
        fprintf(stderr, "%s:%s: \"id\" not found in action item of \"%s\"\n",
                qPrintable(qApp->applicationName()), qPrintable(fileName),
                e.name.toLatin1().data());
        std::exit(1);
    } else {
        result.id = id;
    }

    // type
    determineItemType(fileName, e, result);

    // text
    if (auto text = e.properties.value(QStringLiteral("text")); text.isEmpty()) {
        result.text = itemIdToText(result.id);
    }

    // class
    if (auto commandClass = e.properties.value(QStringLiteral("class")); !commandClass.isEmpty()) {
        result.commandClass = commandClass;
    }

    // shortcuts
    if (auto shortcuts = e.properties.value(QStringLiteral("shortcuts")); !shortcuts.isEmpty()) {
        result.shortcutTokens = parseStringList(shortcuts);
    } else if (shortcuts = e.properties.value(QStringLiteral("shortcut")); !shortcuts.isEmpty()) {
        result.shortcutTokens = parseStringList(shortcuts);
    }

    // categories
    if (auto categories = e.properties.value(QStringLiteral("categories")); !categories.isEmpty()) {
        result.categories = parseStringList(categories);
    } else if (categories = e.properties.value(QStringLiteral("category")); !categories.isEmpty()) {
        result.categories = parseStringList(categories);
    }

    return result;
}

static ActionLayoutMessage parseLayout(const QString &fileName, const QMXmlAdaptorElement &root,
                                       const QStringList &defaultCategory,
                                       QHash<QString, ActionItemInfoMessage> &itemIndexes) {
    ActionLayoutMessage result;
    struct Element {
        QStringList category;
        const QMXmlAdaptorElement *e;
    };
    std::list<Element> stack;
    stack.push_back({defaultCategory, &root});
    while (!stack.empty()) {
        auto top = stack.back();
        stack.pop_back();

        const auto &e = *top.e;
        bool isSep = e.name == QStringLiteral("separator");
        auto id = e.properties.value(QStringLiteral("id"));
        if (id.isEmpty()) {
            fprintf(stderr, "%s:%s: \"id\" not found in layout item of \"%s\"\n",
                    qPrintable(qApp->applicationName()), qPrintable(fileName),
                    e.name.toLatin1().data());
            std::exit(1);
        }

        // Calculate current category
        QStringList currentCategory = top.category;
        
        {
            auto it = itemIndexes.find(id);
            if (it != itemIndexes.end()) {
                // This layout item has been declared in the items field
                auto &info = it.value();

                // Check if the tag matches
                if (info.tag != e.name) {
                    fprintf(stderr,
                            "%s:%s: layout item \"id\" has inconsistent tag \"%s\" with the "
                            "declared one \"%s\"\n",
                            qPrintable(qApp->applicationName()), qPrintable(fileName),
                            e.name.toLatin1().data(), info.tag.toLatin1().data());
                    std::exit(1);
                }

                if (info.categories.isEmpty()) {
                    // The item doesn't have a specified category, use the current one
                    info.categories = top.category;
                } else {
                    currentCategory = info.categories;
                }
                currentCategory += removeAccelerateKeys(info.text);
            } else {
                // Create one
                ActionItemInfoMessage info;
                info.id = id;
                determineItemType(fileName, e, info);
                info.text = itemIdToText(id);
                info.categories = top.category;
                itemIndexes.insert(id, info);
                currentCategory += info.text;
            }
        }

        ActionLayoutMessage::Entry entry;
        entry.id = id;
        entry.flat = e.properties.value(QStringLiteral("flat")) == QStringLiteral("true");

        for (const auto &item : e.children) {
        }
    }
}

static ParserConfig parseParserConfig(const QString &fileName, const QMXmlAdaptorElement &e) {
    Q_UNUSED(fileName);

    ParserConfig result;
    for (const auto &item : e.children) {
        if (item->name == QStringLiteral("defaultCategory")) {
            result.defaultCategory = parseStringList(item->value);
            continue;
        }
    }
    return result;
}

ActionExtensionMessage Parser::parse(const QByteArray &data) const {
    QMXmlAdaptor xml;

    // Read file
    if (!xml.loadData(data)) {
        fprintf(stderr, "%s:%s: invalid format\n", qPrintable(qApp->applicationName()),
                qPrintable(fileName));
        std::exit(1);
    }

    // Check root name
    if (const auto &rootname = xml.root.name; rootname != QStringLiteral("extension")) {
        fprintf(stderr, "%s:%s: unknown element name %s\n", qPrintable(qApp->applicationName()),
                qPrintable(fileName), rootname.toLatin1().data());
        std::exit(1);
    }

    QList<QMXmlAdaptorElement *> itemElements;
    QList<QMXmlAdaptorElement *> layoutElements;
    QList<QMXmlAdaptorElement *> routineElements;

    // Parse root elements and collect attributes
    QString version;
    bool hasParserConfig = false;
    for (const auto &item : std::as_const(xml.root.children)) {
        if (item->name == QStringLiteral("items")) {
            for (const auto &subItem : std::as_const(item->children)) {
                itemElements.append(subItem.data());
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
            parseParserConfig(fileName, *item);
            hasParserConfig = true;
            continue;
        }
    }

    // Build result
    ActionExtensionMessage result;
    result.version = version;
    result.hash = calculateContentSha256(data);

    // Parse items
    QHash<QString, ActionItemInfoMessage> itemIndexes;
    for (const auto &item : std::as_const(itemElements)) {
        auto entity = parseItem(fileName, *item);
        if (itemIndexes.contains(entity.id)) {
            fprintf(stderr, "%s:%s: duplicated item id %s\n", qPrintable(qApp->applicationName()),
                    qPrintable(fileName), entity.id.toLatin1().data());
            std::exit(1);
        }
        result.items.append(entity);
    }

    return {};
}
