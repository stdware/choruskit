#ifndef PARSER_H
#define PARSER_H

#include <QtCore/QFile>
#include <QtCore/QHash>

struct ActionObjectInfoMessage {
    QString id;
    QString typeToken;
    QString text;
    QString commandClass;
    QStringList shortcutTokens;
    QStringList categories;
    bool topLevel = false;

    // Metadata
    QString tag;
};

struct ActionLayoutEntryMessage {
    QString id;
    QString typeToken;
    bool flat = false;
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
