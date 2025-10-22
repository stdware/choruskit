#ifndef CHORUSKIT_FILELOCKER_P_H
#define CHORUSKIT_FILELOCKER_P_H

#include <CoreApi/filelocker.h>

#include <memory>

#include <QFile>

namespace Core {

    class FileLockerPrivate {
        Q_DECLARE_PUBLIC(FileLocker)
    public:
        FileLocker *q_ptr;
        
        QString filePath;
        bool isReadOnly = false;
        std::unique_ptr<QFile> saveFile;
        std::unique_ptr<QFile> readFile;
        QString errorString;

        void cleanup();
        bool openForReadWrite(const QString &path);
        bool openForReadOnly(const QString &path);
    };

}

#endif // CHORUSKIT_FILELOCKER_P_H