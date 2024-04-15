#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <qmxmladaptor.h>

#include "parser.h"
#include "generator.h"

void error(const char *msg = "Invalid argument") {
    if (msg)
        fprintf(stderr, "%s:%s\n", qPrintable(qApp->applicationName()), msg);
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationVersion(QString::fromLatin1(APP_VERSION));

    // Build command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("ChorusKit Action Extension Compiler version %1 (Qt %2)")
            .arg(QString::fromLatin1(APP_VERSION), QString::fromLatin1(QT_VERSION_STR)));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    QCommandLineOption outputOption(QStringLiteral("o"));
    outputOption.setDescription(QStringLiteral("Write output to file rather than stdout."));
    outputOption.setValueName(QStringLiteral("file"));
    outputOption.setFlags(QCommandLineOption::ShortOptionStyle);
    parser.addOption(outputOption);

    QCommandLineOption identifierOption(QStringLiteral("i"));
    identifierOption.setDescription(
        QStringLiteral("Override extension identifier rather than the file name."));
    identifierOption.setValueName(QStringLiteral("identifier"));
    parser.addOption(identifierOption);

    QCommandLineOption defineOption(QStringLiteral("D"));
    defineOption.setDescription(QStringLiteral("Define a variable."));
    defineOption.setValueName(QStringLiteral("macro[=def]"));
    defineOption.setFlags(QCommandLineOption::ShortOptionStyle);
    parser.addOption(defineOption);

    parser.addPositionalArgument(QStringLiteral("<file>"),
                                 QStringLiteral("Manifest file to read from."));

    if (argc == 1) {
        parser.showHelp(0);
    }
    parser.process(QCoreApplication::arguments());

    // Parse command line arguments
    Parser pp;
    QString filename;
    if (const QStringList files = parser.positionalArguments(); files.count() > 1) {
        error(qPrintable(QLatin1String("Too many input files specified: '") +
                         files.join(QLatin1String("' '")) + QLatin1Char('\'')));
        parser.showHelp(1);
    } else if (files.isEmpty()) {
        error(qPrintable(QLatin1String("Input file not specified.")));
        parser.showHelp(1);
    } else {
        filename = files.first();
        pp.fileName = filename;
    }

    for (const QString &arg : parser.values(defineOption)) {
        QByteArray name = arg.toLocal8Bit();
        QByteArray value = name;
        int eq = name.indexOf('=');
        if (eq >= 0) {
            value = name.mid(eq + 1);
            name = name.left(eq);
        }
        if (name.isEmpty()) {
            error("Missing macro name");
            parser.showHelp(1);
        }
        pp.variables.insert(QString::fromLatin1(name), QString::fromLatin1(value));
    }

    QString identifier = parser.value(identifierOption);
    if (identifier.isEmpty()) {
        identifier = QFileInfo(filename).baseName();
    }

    QString output = parser.value(outputOption);

    // Parse XML file
    QFile in;
    in.setFileName(filename);
    if (!in.open(QIODevice::ReadOnly)) {
        fprintf(stderr, "%s:%s: No such file\n", qPrintable(qApp->applicationName()),
                qPrintable(filename));
        return 1;
    }

    // If there's error, the program will exit right away.
    auto extensionMessage = pp.parse(in.readAll());

    // Generate
    FILE *out;
    if (!output.isEmpty()) {
#if defined(_MSC_VER)
        if (_wfopen_s(&out, reinterpret_cast<const wchar_t *>(output.utf16()), L"w") != 0)
#else
        out = fopen(QFile::encodeName(output).constData(), "w"); // create output file
        if (!out)
#endif
        {
            fprintf(stderr, "%s:Cannot create %s\n", qPrintable(qApp->applicationName()),
                    QFile::encodeName(output).constData());
            return 1;
        }
    } else {
        out = stdout;
    }

    Generator generator(out, QFileInfo(filename).fileName().toLocal8Bit(), identifier.toLocal8Bit(),
                        extensionMessage);
    generator.generateCode();

    if (!output.isEmpty())
        fclose(out);

    return 0;
}