#ifndef PARSER_H
#define PARSER_H

#include <QtCore/QFile>
#include <QtCore/QHash>

struct ActionObjectInfoMessage {
    enum Type {
        Action,
        Group,
        Menu,
        ExpandedMenu,
        Separator,
        Stretch,
    };
    enum Mode {
        Plain,
        Unique,
        Widget,
        TopLevel,
    };
    QString id;
    Type type;
    Mode mode;
    QString text;
    QString commandClass;
    QStringList shortcutTokens;
    QStringList categories;

    // Metadata
    QString tag;

    static QString typeToString(Type type);
    static QString modeToString(Mode mode);
};

struct ActionLayoutEntryMessage {
    QString id;
    ActionObjectInfoMessage::Type type;
    QVector<int> childIndexes;
};

struct ActionBuildRoutineMessage {
    QString anchorToken;
    QString parent;
    QString relativeTo;
    QVector<int> entryIndexes;
};

struct ActionExtensionMessage {
    QString hash;
    QString version;

    QVector<ActionObjectInfoMessage> objects;
    QVector<ActionLayoutEntryMessage> layouts;
    QVector<int> layoutRootIndexes;
    QVector<ActionBuildRoutineMessage> buildRoutines;
};

class Parser {
public:
    Parser();

    QString fileName;
    QHash<QString, QString> variables;

    ActionExtensionMessage parse(const QByteArray &data) const;
};

#endif // PARSER_H
