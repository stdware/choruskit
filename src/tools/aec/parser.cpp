#include "parser.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QRegularExpression>

void error(const char *msg);

static QString calculateContentSha256(const QByteArray &data) {
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(data);
    return hash.result().toHex();
}

static const char DefaultPattern[] = R"(\$\{(\w+)\})";

static QString parseExpression(QString s, const QRegularExpression &reg,
                               const QHash<QString, QString> &vars, bool recursive = true) {
    QRegularExpressionMatch match;
    int index = 0;
    bool hasMatch = false;
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
    return parseExpression(s, reg, vars);
}

Parser::Parser() {
}

ActionExtensionMessage Parser::parse(const QByteArray &data) const {
    // TODO
    return ActionExtensionMessage();
}
