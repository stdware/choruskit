#include <QtCore/QCoreApplication>
#include <QtCore/QRegularExpression>
#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>

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

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationVersion(APP_VERSION);

    QT_VERSION_STR;

    QCommandLineParser parser;
    parser.parse(QCoreApplication::arguments());

    QByteArray fn;
    FILE *out = nullptr;
    if (out) {
        fprintf(out,
                "/****************************************************************************\n"
                "** Meta object code from reading XML file '%s'\n**\n",
                fn.constData());
        fprintf(out, "** Created by: ChorusKit Action Extension Compiler version %s (Qt %s)\n**\n",
                APP_VERSION, QT_VERSION_STR);
        fprintf(
            out,
            "** WARNING! All changes made in this file will be lost!\n"
            "*****************************************************************************/\n\n");
    }
    return 0;
}