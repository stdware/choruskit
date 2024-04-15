#ifndef PARSER_H
#define PARSER_H

#include <QtCore/QFile>
#include <QtCore/QHash>

struct ActionItemInfoMessage {
    QString id;
    QString typeToken;
    QByteArray text;
    QByteArray commandClass;
    QStringList shortcutTokens;
    QByteArrayList categories;
    bool topLevel;
};

struct ActionLayoutMessage {
    struct Entry {
        QString id;
        QString typeToken;
        bool flat;
        QVector<int> childIndexes;
    };
    QVector<Entry> entryData;
};

struct ActionBuildRoutineMessage {
    QString anchorToken;
    QString parent;
    QString relativeTo;

    struct Item {
        QString id;
        QString typeToken;
        bool flat;
    };
    QVector<Item> items;
};

struct ActionExtensionMessage {
    QString hash;
    QString version;

    QVector<ActionItemInfoMessage> items;
    QVector<ActionLayoutMessage> layouts;
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
