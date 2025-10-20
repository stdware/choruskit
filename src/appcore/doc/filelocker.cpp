#include "filelocker.h"
#include "filelocker_p.h"

#include <QFileInfo>
#include <QLoggingCategory>
#include <QIODevice>

namespace Core {

    Q_STATIC_LOGGING_CATEGORY(lcFileLocker, "ck.filelocker")

    void FileLockerPrivate::init() {
        // Initialize private data
        saveFile = nullptr;
        readFile = nullptr;
        filePath.clear();
        isReadOnly = false;
    }

    void FileLockerPrivate::cleanup() {
        if (saveFile) {
            if (saveFile->isOpen()) {
                saveFile->cancelWriting();
            }
            delete saveFile;
            saveFile = nullptr;
        }
        
        if (readFile) {
            if (readFile->isOpen()) {
                readFile->close();
            }
            delete readFile;
            readFile = nullptr;
        }
        
        filePath.clear();
        isReadOnly = false;
    }

    bool FileLockerPrivate::openForReadWrite(const QString &path) {
        Q_Q(FileLocker);
        
        // Try to open with QSaveFile for read-write access
        saveFile = new QSaveFile(path);
        if (saveFile->open(QIODevice::ReadWrite)) {
            filePath = path;
            isReadOnly = false;
            qCDebug(lcFileLocker) << "Opened file for read-write access:" << path;
            return true;
        }
        
        // Failed to open for write, cleanup saveFile
        delete saveFile;
        saveFile = nullptr;
        qCDebug(lcFileLocker) << "Failed to open file for read-write access:" << path;
        return false;
    }

    bool FileLockerPrivate::openForReadOnly(const QString &path) {
        Q_Q(FileLocker);
        
        // Try to open with QFile for read-only access
        readFile = new QFile(path);
        if (readFile->open(QIODevice::ReadOnly)) {
            filePath = path;
            isReadOnly = true;
            qCDebug(lcFileLocker) << "Opened file for read-only access:" << path;
            return true;
        }
        
        // Failed to open for read, cleanup readFile
        delete readFile;
        readFile = nullptr;
        qCWarning(lcFileLocker) << "Failed to open file for read-only access:" << path;
        return false;
    }

    FileLocker::FileLocker(QObject *parent)
        : QObject(parent), d_ptr(new FileLockerPrivate) {
        Q_D(FileLocker);
        d->q_ptr = this;
        d->init();
    }

    FileLocker::~FileLocker() {
        Q_D(FileLocker);
        d->cleanup();
    }

    QString FileLocker::path() const {
        Q_D(const FileLocker);
        return d->filePath;
    }

    bool FileLocker::isReadOnly() const {
        Q_D(const FileLocker);
        return d->isReadOnly;
    }

    bool FileLocker::open(const QString &path) {
        Q_D(FileLocker);
        
        // Check if file exists
        QFileInfo fileInfo(path);
        if (!fileInfo.exists()) {
            qCWarning(lcFileLocker) << "File does not exist:" << path;
            return false;
        }
        
        qCDebug(lcFileLocker) << "Attempting to open file:" << path;
        
        // Close any currently opened file
        close();
        
        // First try to open for read-write
        if (d->openForReadWrite(path)) {
            emit pathChanged();
            emit readOnlyChanged();
            return true;
        }
        
        // If read-write failed, try read-only
        if (d->openForReadOnly(path)) {
            emit pathChanged();
            emit readOnlyChanged();
            return true;
        }
        
        qCWarning(lcFileLocker) << "Failed to open file:" << path;
        return false;
    }

    QByteArray FileLocker::readData() {
        Q_D(FileLocker);
        
        if (d->filePath.isEmpty()) {
            qCWarning(lcFileLocker) << "No file is currently open";
            return QByteArray();
        }
        
        qCDebug(lcFileLocker) << "Reading data from file:" << d->filePath;
        
        QByteArray result;
        
        if (d->saveFile && d->saveFile->isOpen()) {
            // For QSaveFile, we need to read from original file
            QFile originalFile(d->filePath);
            if (originalFile.open(QIODevice::ReadOnly)) {
                result = originalFile.readAll();
                originalFile.close();
                qCDebug(lcFileLocker) << "Read" << result.size() << "bytes from file (via QSaveFile)";
            } else {
                qCWarning(lcFileLocker) << "Failed to read from original file:" << d->filePath;
            }
        } else if (d->readFile && d->readFile->isOpen()) {
            // For QFile, seek to beginning and read all
            if (d->readFile->seek(0)) {
                result = d->readFile->readAll();
                qCDebug(lcFileLocker) << "Read" << result.size() << "bytes from file (via QFile)";
            } else {
                qCWarning(lcFileLocker) << "Failed to seek to beginning of file:" << d->filePath;
            }
        } else {
            qCWarning(lcFileLocker) << "No valid file handle available for reading";
        }
        
        return result;
    }

    bool FileLocker::save(const QByteArray &data) {
        Q_D(FileLocker);
        
        if (d->filePath.isEmpty()) {
            qCWarning(lcFileLocker) << "No file is currently open";
            return false;
        }
        
        if (d->isReadOnly) {
            qCWarning(lcFileLocker) << "Cannot save to read-only file:" << d->filePath;
            return false;
        }
        
        if (!d->saveFile || !d->saveFile->isOpen()) {
            qCWarning(lcFileLocker) << "No writable file handle available";
            return false;
        }
        
        qCDebug(lcFileLocker) << "Saving" << data.size() << "bytes to file:" << d->filePath;
        
        // Seek to beginning and resize to 0 to overwrite
        if (!d->saveFile->seek(0)) {
            qCWarning(lcFileLocker) << "Failed to seek to beginning of file:" << d->filePath;
            return false;
        }
        
        if (!d->saveFile->resize(0)) {
            qCWarning(lcFileLocker) << "Failed to truncate file:" << d->filePath;
            return false;
        }
        
        // Write the data
        qint64 bytesWritten = d->saveFile->write(data);
        if (bytesWritten != data.size()) {
            qCWarning(lcFileLocker) << "Failed to write all data to file. Expected:" << data.size() << "Actual:" << bytesWritten;
            return false;
        }
        
        // Commit the changes
        if (!d->saveFile->commit()) {
            qCWarning(lcFileLocker) << "Failed to commit changes to file:" << d->filePath;
            return false;
        }
        
        qCDebug(lcFileLocker) << "Successfully saved data to file:" << d->filePath;
        
        // Reopen the file for further operations
        QString currentPath = d->filePath;
        d->cleanup();
        return d->openForReadWrite(currentPath);
    }

    bool FileLocker::saveAs(const QString &path, const QByteArray &data) {
        Q_D(FileLocker);
        
        qCDebug(lcFileLocker) << "Saving" << data.size() << "bytes to new file:" << path;
        
        // Create a temporary QSaveFile to write to the new path
        QSaveFile newFile(path);
        if (!newFile.open(QIODevice::WriteOnly)) {
            qCWarning(lcFileLocker) << "Failed to open new file for writing:" << path;
            return false;
        }
        
        // Write the data
        qint64 bytesWritten = newFile.write(data);
        if (bytesWritten != data.size()) {
            qCWarning(lcFileLocker) << "Failed to write all data to new file. Expected:" << data.size() << "Actual:" << bytesWritten;
            newFile.cancelWriting();
            return false;
        }
        
        // Commit the new file
        if (!newFile.commit()) {
            qCWarning(lcFileLocker) << "Failed to commit new file:" << path;
            return false;
        }
        
        qCDebug(lcFileLocker) << "Successfully saved data to new file:" << path;
        
        // Close current file and open the new one
        close();
        
        if (d->openForReadWrite(path)) {
            emit pathChanged();
            emit readOnlyChanged();
            return true;
        }
        
        qCWarning(lcFileLocker) << "Failed to reopen new file after save:" << path;
        return false;
    }

    void FileLocker::close() {
        Q_D(FileLocker);
        
        if (!d->filePath.isEmpty()) {
            qCDebug(lcFileLocker) << "Closing file:" << d->filePath;
        }
        
        bool pathChanged = !d->filePath.isEmpty();
        bool readOnlyChanged = d->isReadOnly;
        
        d->cleanup();
        
        if (pathChanged) {
            emit this->pathChanged();
        }
        
        if (readOnlyChanged) {
            emit this->readOnlyChanged();
        }
    }

}

#include "moc_filelocker.cpp"