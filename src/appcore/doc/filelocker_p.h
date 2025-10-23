#ifndef CHORUSKIT_FILELOCKER_P_H
#define CHORUSKIT_FILELOCKER_P_H

#include <CoreApi/filelocker.h>

#include <memory>

#include <QFile>
#include <QFileSystemWatcher>

namespace Core {

    class FileLockerPrivate {
        Q_DECLARE_PUBLIC(FileLocker)
    public:
        FileLocker *q_ptr;
        QString filePath;
        std::unique_ptr<QFile> file;
        std::unique_ptr<QFileSystemWatcher> watcher;
        QString errorString;
        bool isFileModifiedSinceLastSave{};
    };

}

#endif // CHORUSKIT_FILELOCKER_P_H