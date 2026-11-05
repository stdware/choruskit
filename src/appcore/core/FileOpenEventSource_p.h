#ifndef CHORUSKIT_FILEOPENEVENTSOURCE_P_H
#define CHORUSKIT_FILEOPENEVENTSOURCE_P_H

#include <CoreApi/FileOpenEventSource.h>

namespace Core {

    class FileOpenEventSourcePrivate {
        Q_DECLARE_PUBLIC(FileOpenEventSource)
    public:
        FileOpenEventSource *q_ptr = nullptr;
        QStringList pendingFiles;
    };

}

#endif // CHORUSKIT_FILEOPENEVENTSOURCE_P_H
