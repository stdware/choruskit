#include "generator.h"

void error(const char *msg);

Generator::Generator(FILE *out, const QByteArray &inputFileName, const QByteArray &identifier,
                     const ActionExtensionMessage &message)
    : out(out), inputFileName(inputFileName), identifier(identifier), msg(message) {
}

void Generator::generateCode() {
    fprintf(out,
            "/****************************************************************************\n"
            "** Meta object code from reading XML file '%s'\n**\n",
            inputFileName.constData());
    fprintf(out, "** Created by: ChorusKit Action Extension Compiler version %s (Qt %s)\n**\n",
            APP_VERSION, QT_VERSION_STR);
    fprintf(out, "** WARNING! All changes made in this file will be lost!\n"
                 "*************************************************************************"
                 "****/\n\n");
    // TODO
}
