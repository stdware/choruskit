#ifndef CHORUSKIT_FILELOCKER_P_H
#define CHORUSKIT_FILELOCKER_P_H

#include "filelocker.h"

#include <QSaveFile>
#include <QFile>

namespace Core {

    class FileLockerPrivate {
        Q_DECLARE_PUBLIC(FileLocker)
    public:
        FileLocker *q_ptr;
        
        QString filePath;
        bool isReadOnly = false;
        QSaveFile *saveFile = nullptr;
        QFile *readFile = nullptr;
        
        void init();
        void cleanup();
        bool openForReadWrite(const QString &path);
        bool openForReadOnly(const QString &path);
    };

}

#endif // CHORUSKIT_FILELOCKER_P_H