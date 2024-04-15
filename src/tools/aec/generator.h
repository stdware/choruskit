#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"

class Generator {
public:
    Generator(FILE *out, const QByteArray &inputFileName, const QByteArray &identifier,
              const ActionExtensionMessage &message);

    void generateCode();

private:
    FILE *out;
    QByteArray inputFileName;
    QByteArray identifier;
    ActionExtensionMessage msg;
};

#endif // GENERATOR_H
