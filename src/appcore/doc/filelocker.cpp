#include "filelocker.h"
#include "filelocker_p.h"

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QIODevice>

namespace Core {

    Q_STATIC_LOGGING_CATEGORY(lcFileLocker, "ck.filelocker")

    void FileLockerPrivate::cleanup() {
        if (saveFile && saveFile->isOpen()) {
            saveFile->close();
        }
        saveFile.reset();
        
        if (readFile && readFile->isOpen()) {
            readFile->close();
        }
        readFile.reset();
        
        filePath.clear();
        isReadOnly = false;
    }

    bool FileLockerPrivate::openForReadWrite(const QString &path) {
        Q_Q(FileLocker);
        
        // Try to open with QSaveFile for read-write access
        errorString.clear();
        auto saveFile_ = std::make_unique<QFile>(path);
        if (saveFile_->open(QIODevice::ReadWrite)) {
            filePath = path;
            isReadOnly = false;
            qCDebug(lcFileLocker) << "Opened file for read-write access:" << path;
            saveFile = std::move(saveFile_);
            return true;
        }
        
        // Failed to open for write
        qCDebug(lcFileLocker) << "Failed to open file for read-write access:" << path << saveFile_->error() << saveFile_->errorString();
        errorString = saveFile_->errorString();
        return false;
    }

    bool FileLockerPrivate::openForReadOnly(const QString &path) {
        Q_Q(FileLocker);
        
        // Try to open with QFile for read-only access
        errorString.clear();
        auto readFile_ = std::make_unique<QFile>(path);
        if (readFile_->open(QIODevice::ReadOnly)) {
            filePath = path;
            isReadOnly = true;
            qCDebug(lcFileLocker) << "Opened file for read-only access:" << path;
            readFile = std::move(readFile_);
            return true;
        }
        
        // Failed to open for read
        qCWarning(lcFileLocker) << "Failed to open file for read-only access:" << path << readFile_->error() << readFile_->errorString();
        errorString = readFile_->errorString();
        return false;
    }

    FileLocker::FileLocker(QObject *parent)
        : QObject(parent), d_ptr(new FileLockerPrivate) {
        Q_D(FileLocker);
        d->q_ptr = this;
    }

    FileLocker::~FileLocker() {
        Q_D(FileLocker);
        d->cleanup();
    }

    QString FileLocker::path() const {
        Q_D(const FileLocker);
        return QDir::toNativeSeparators(d->filePath);
    }
    
    QString FileLocker::entryName() const {
        Q_D(const FileLocker);
        return QFileInfo(d->filePath).fileName();
    }

    bool FileLocker::isReadOnly() const {
        Q_D(const FileLocker);
        return d->isReadOnly;
    }

    QString FileLocker::errorString() const {
        Q_D(const FileLocker);
        return d->errorString;
    }

    bool FileLocker::open(const QString &path) {
        Q_D(FileLocker);
        
        // Check if file exists
        QFileInfo fileInfo(path);
        if (!fileInfo.exists()) {
            qCWarning(lcFileLocker) << "File does not exist:" << path;
            d->errorString = tr("File does not exist");
            return false;
        }

        qCDebug(lcFileLocker) << "Attempting to open file:" << path;
        
        // Close any currently opened file
        close();
        
        // First try to open for read-write
        if (d->openForReadWrite(path)) {
            Q_EMIT pathChanged();
            Q_EMIT entryNameChanged();
            Q_EMIT readOnlyChanged();
            return true;
        }
        
        // If read-write failed, try read-only
        if (d->openForReadOnly(path)) {
            Q_EMIT pathChanged();
            Q_EMIT entryNameChanged();
            Q_EMIT readOnlyChanged();
            return true;
        }
        
        qCWarning(lcFileLocker) << "Failed to open file:" << path;
        return false;
    }

    QByteArray FileLocker::readData(bool *ok) {
        Q_D(FileLocker);

        d->errorString.clear();
        
        if (d->filePath.isEmpty()) {
            qCWarning(lcFileLocker) << "No file is currently open";
            if (ok)
                *ok = false;
            return {};
        }
        
        qCDebug(lcFileLocker) << "Reading data from file:" << d->filePath;
        
        QByteArray result;
        
        if (d->saveFile && d->saveFile->isOpen()) {
            if (d->saveFile->seek(0)) {
                result = d->saveFile->readAll();
                qCDebug(lcFileLocker) << "Read" << result.size() << "bytes from file (via QSaveFile)";
            } else {
                qCWarning(lcFileLocker) << "Failed to seek to beginning of file:" << d->filePath << d->saveFile->error() << d->saveFile->errorString();
                d->errorString = d->saveFile->errorString();
                if (ok)
                    *ok = false;
            }
        } else if (d->readFile && d->readFile->isOpen()) {
            // For QFile, seek to beginning and read all
            if (d->readFile->seek(0)) {
                result = d->readFile->readAll();
                qCDebug(lcFileLocker) << "Read" << result.size() << "bytes from file (via QFile)";
            } else {
                qCWarning(lcFileLocker) << "Failed to seek to beginning of file:" << d->filePath << d->readFile->error() << d->readFile->errorString();
                d->errorString = d->readFile->errorString();
                if (ok)
                    *ok = false;
            }
        } else {
            qCWarning(lcFileLocker) << "No valid file handle available for reading";
            if (ok)
                *ok = false;
        }

        if (ok)
            *ok = true;
        
        return result;
    }

    bool FileLocker::save(const QByteArray &data) {
        Q_D(FileLocker);

        d->errorString.clear();
        
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
            qCWarning(lcFileLocker) << "Failed to seek to beginning of file:" << d->filePath << d->saveFile->error() << d->saveFile->errorString();
            d->errorString = d->saveFile->errorString();
            return false;
        }
        
        if (!d->saveFile->resize(0)) {
            qCWarning(lcFileLocker) << "Failed to truncate file:" << d->filePath << d->saveFile->error() << d->saveFile->errorString();
            d->errorString = d->saveFile->errorString();
            return false;
        }
        
        // Write the data
        qint64 bytesWritten = d->saveFile->write(data);
        if (bytesWritten != data.size()) {
            qCWarning(lcFileLocker) << "Failed to write all data to file. Expected:" << data.size() << "Actual:" << bytesWritten;
            d->errorString = d->saveFile->errorString();
            return false;
        }
        
        qCDebug(lcFileLocker) << "Successfully saved data to file:" << d->filePath;
        return true;
    }

    bool FileLocker::saveAs(const QString &path, const QByteArray &data) {
        Q_D(FileLocker);

        d->errorString.clear();
        
        qCDebug(lcFileLocker) << "Saving" << data.size() << "bytes to new file:" << path;
        
        // Create a temporary QSaveFile to write to the new path
        auto newFile = std::make_unique<QFile>(path);
        if (!newFile->open(QIODevice::ReadWrite)) {
            qCWarning(lcFileLocker) << "Failed to open new file for writing:" << path << newFile->error() << newFile->errorString();
            d->errorString = newFile->errorString();
            return false;
        }
        
        // Write the data
        qint64 bytesWritten = newFile->write(data);
        if (bytesWritten != data.size()) {
            qCWarning(lcFileLocker) << "Failed to write all data to new file. Expected:" << data.size() << "Actual:" << bytesWritten;
            d->errorString = newFile->errorString();
            return false;
        }
        
        qCDebug(lcFileLocker) << "Successfully saved data to new file:" << path;
        
        d->saveFile = std::move(newFile);
        d->filePath = path;
        d->isReadOnly = false;
        Q_EMIT pathChanged();
        Q_EMIT entryNameChanged();
        Q_EMIT readOnlyChanged();
        return true;
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
            Q_EMIT this->pathChanged();
            Q_EMIT this->entryNameChanged();
        }
        
        if (readOnlyChanged) {
            Q_EMIT this->readOnlyChanged();
        }
    }

}

#include "moc_filelocker.cpp"