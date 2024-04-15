#include "generator.h"

void error(const char *msg);

Generator::Generator(FILE *out, const QByteArray &inputFileName, const QByteArray &identifier,
                     const ActionExtensionMessage &message)
    : out(out), inputFileName(inputFileName), identifier(identifier), msg(message) {
}

void Generator::generateCode() {
    // Warning
    fprintf(out,
            "/****************************************************************************\n"
            "** Meta object code from reading XML file '%s'\n**\n",
            inputFileName.constData());
    fprintf(out, "** Created by: ChorusKit Action Extension Compiler version %s (Qt %s)\n**\n",
            APP_VERSION, QT_VERSION_STR);
    fprintf(out, "** WARNING! All changes made in this file will be lost!\n"
                 "*************************************************************************"
                 "****/\n\n");

    // Headers
    fprintf(out, "#include <QtCore/QCoreApplication>\n");
    fprintf(out, "\n");
    fprintf(out, "#include <CoreApi/private/actionextension_p.h>\n");

    // Generate
    // fprintf(out, "    data.hash = QStringLiteral(\"%s\");", msg.hash.toLatin1().data());
}
