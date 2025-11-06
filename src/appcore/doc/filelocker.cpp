#include "filelocker.h"
#include "filelocker_p.h"

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QIODevice>
#include <QFileSystemWatcher>

namespace Core {

    Q_STATIC_LOGGING_CATEGORY(lcFileLocker, "ck.filelocker")

    FileLocker::FileLocker(QObject *parent)
        : QObject(parent), d_ptr(new FileLockerPrivate) {
        Q_D(FileLocker);
        d->q_ptr = this;
        d->watcher = new QFileSystemWatcher(this);
        
        // Connect file system watcher signal
        connect(d->watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &path) {
            Q_D(FileLocker);
            Q_UNUSED(path)
            qCDebug(lcFileLocker) << "File" << d->filePath << "has been changed externally";
            if (!d->isFileModifiedSinceLastSave) {
                d->isFileModifiedSinceLastSave = true;
                Q_EMIT fileModifiedSinceLastSaveChanged();
            }
        });
    }

    FileLocker::~FileLocker() {
        Q_D(FileLocker);

    }

    QString FileLocker::path() const {
        Q_D(const FileLocker);
        return QDir::toNativeSeparators(d->filePath);
    }
    
    QString FileLocker::entryName() const {
        Q_D(const FileLocker);
        return QFileInfo(d->filePath).fileName();
    }

    QString FileLocker::errorString() const {
        Q_D(const FileLocker);
        return d->errorString;
    }

    bool FileLocker::isFileModifiedSinceLastSave() const {
        Q_D(const FileLocker);
        return d->isFileModifiedSinceLastSave;
    }

    bool FileLocker::open(const QString &path) {
        Q_D(FileLocker);
        
        // Close current file if open
        if (d->file && d->file->isOpen()) {
            d->file->close();
        }
        
        // Remove previous file from watcher
        if (!d->filePath.isEmpty() && d->watcher->files().contains(d->filePath)) {
            d->watcher->removePath(d->filePath);
        }

        d->filePath.clear();
        d->errorString.clear();
        
        // Create new file object
        d->file = std::make_unique<QFile>(path);
        
        // Try to open in ReadWrite mode first
        if (d->file->open(QIODevice::ReadWrite | QIODevice::ExistingOnly)) {
            qCDebug(lcFileLocker) << "Opened file in ReadWrite mode:" << path;
        } else if (d->file->open(QIODevice::ReadOnly | QIODevice::ExistingOnly)) {
            qCDebug(lcFileLocker) << "Opened file in ReadOnly mode:" << path;
        } else {
            // Failed to open file
            d->errorString = d->file->errorString();
            qCWarning(lcFileLocker) << "Failed to open file:" << path << "Error:" << d->errorString;
            d->file.reset();
            return false;
        }
        
        // Update file path
        d->filePath = path;
        
        // Add to file system watcher
        d->watcher->addPath(path);
        
        // Reset modification flag
        if (d->isFileModifiedSinceLastSave) {
            d->isFileModifiedSinceLastSave = false;
            Q_EMIT fileModifiedSinceLastSaveChanged();
        }
        
        Q_EMIT pathChanged();
        Q_EMIT entryNameChanged();
        
        return true;
    }

    QByteArray FileLocker::readData(bool *ok) {
        Q_D(FileLocker);
        
        if (ok) {
            *ok = false;
        }

        d->errorString.clear();
        
        if (!d->file || !d->file->isOpen()) {
            qCWarning(lcFileLocker) << "Attempted to read data when no file is open";
            return {};
        }
        
        // Save current position
        qint64 originalPos = d->file->pos();
        
        // Seek to beginning
        if (!d->file->seek(0)) {
            d->errorString = d->file->errorString();
            qCWarning(lcFileLocker) << "Failed to seek to beginning of file:" << d->errorString;
            return {};
        }
        
        // Read all data
        QByteArray data = d->file->readAll();
        if (d->file->error() != QFile::NoError) {
            d->errorString = d->file->errorString();
            qCWarning(lcFileLocker) << "Failed to read file data:" << d->errorString;
            return {};
        }
        
        if (ok) {
            *ok = true;
        }
        
        qCDebug(lcFileLocker) << "Successfully read" << data.size() << "bytes from file";
        return data;
    }

    void FileLocker::release() {
        Q_D(FileLocker);
        
        if (d->file && d->file->isOpen()) {
            d->file->close();
            qCDebug(lcFileLocker) << "Released file:" << d->filePath;
        }
        
        // Keep the file path but reset the file object
        d->file.reset();
        
        // Clear error string
        d->errorString.clear();
    }

    bool FileLocker::save(const QByteArray &data) {
        Q_D(FileLocker);

        d->errorString.clear();
        
        if (d->filePath.isEmpty()) {
            qCWarning(lcFileLocker) << "Attempted to save when no file path is set";
            return false;
        }

        auto reAddPath = [&](void *) {
            d->watcher->addPath(d->filePath);
        };
        std::unique_ptr<void, decltype(reAddPath)> guard(this, reAddPath);
        d->watcher->removePath(d->filePath);
        
        if (d->file && d->file->isOpen()) {
            // File is open, check if we can write
            if (d->file->openMode() & QIODevice::WriteOnly) {
                // File is writable, write directly
                if (!d->file->seek(0)) {
                    d->errorString = d->file->errorString();
                    qCWarning(lcFileLocker) << "Failed to seek to beginning for save:" << d->errorString;
                    return false;
                }
                
                if (d->file->write(data) != data.size()) {
                    d->errorString = d->file->errorString();
                    qCWarning(lcFileLocker) << "Failed to write data:" << d->errorString;
                    return false;
                }
                
                // Truncate file at current position
                if (!d->file->resize(d->file->pos())) {
                    d->errorString = d->file->errorString();
                    qCWarning(lcFileLocker) << "Failed to resize file:" << d->errorString;
                    return false;
                }
                
                if (!d->file->flush()) {
                    d->errorString = d->file->errorString();
                    qCWarning(lcFileLocker) << "Failed to flush file:" << d->errorString;
                    return false;
                }
            } else {
                // File is ReadOnly, try to reopen as ReadWrite
                auto tempFile = std::make_unique<QFile>(d->filePath);
                
                if (!tempFile->open(QIODevice::ReadWrite)) {
                    // Failed to reopen as ReadWrite, restore ReadOnly mode
                    d->errorString = tempFile->errorString();
                    qCWarning(lcFileLocker) << "Failed to reopen file as ReadWrite:" << d->errorString;
                    return false;
                }

                if (!tempFile->seek(0)) {
                    d->errorString = tempFile->errorString();
                    qCWarning(lcFileLocker) << "Failed to seek after reopen:" << d->errorString;
                    return false;
                }

                if (tempFile->write(data) != data.size()) {
                    d->errorString = tempFile->errorString();
                    qCWarning(lcFileLocker) << "Failed to write after reopen:" << d->errorString;
                    return false;
                }

                if (!tempFile->resize(tempFile->pos())) {
                    d->errorString = tempFile->errorString();
                    qCWarning(lcFileLocker) << "Failed to resize after reopen:" << d->errorString;
                    return false;
                }

                if (!tempFile->flush()) {
                    d->errorString = tempFile->errorString();
                    qCWarning(lcFileLocker) << "Failed to flush after reopen:" << d->errorString;
                    return false;
                }

                d->file = std::move(tempFile);
            }
        } else {
            // No file is open, open the file, write data, and close immediately
            QFile tempFile(d->filePath);
            if (!tempFile.open(QIODevice::WriteOnly)) {
                d->errorString = tempFile.errorString();
                qCWarning(lcFileLocker) << "Failed to open file for saving:" << d->errorString;
                return false;
            }
            
            if (tempFile.write(data) != data.size()) {
                d->errorString = tempFile.errorString();
                qCWarning(lcFileLocker) << "Failed to write data to file:" << d->errorString;
                return false;
            }

            if (!tempFile.resize(tempFile.pos())) {
                d->errorString = tempFile.errorString();
                qCWarning(lcFileLocker) << "Failed to resize after reopen:" << d->errorString;
                return false;
            }
            
            if (!tempFile.flush()) {
                d->errorString = tempFile.errorString();
                qCWarning(lcFileLocker) << "Failed to flush temporary file:" << d->errorString;
                return false;
            }
            
            tempFile.close();
        }
        
        // Reset modification flag after successful save
        if (d->isFileModifiedSinceLastSave) {
            d->isFileModifiedSinceLastSave = false;
            Q_EMIT fileModifiedSinceLastSaveChanged();
        }
        
        qCDebug(lcFileLocker) << "Successfully saved" << data.size() << "bytes to file:" << d->filePath;
        return true;
    }

    bool FileLocker::saveAs(const QString &path, const QByteArray &data) {
        Q_D(FileLocker);

        d->errorString.clear();
        
        if (path.isEmpty()) {
            qCWarning(lcFileLocker) << "Attempted saveAs with empty path";
            return false;
        }
        
        // Create a new file for the saveAs operation
        auto newFile = std::make_unique<QFile>(path);
        
        if (!newFile->open(QIODevice::ReadWrite)) {
            d->errorString = newFile->errorString();
            qCWarning(lcFileLocker) << "Failed to open file for saveAs:" << path << "Error:" << d->errorString;
            return false;
        }

        if (newFile->write(data) != data.size()) {
            d->errorString = newFile->errorString();
            qCWarning(lcFileLocker) << "Failed to write data during saveAs:" << d->errorString;
            newFile->close();
            return false;
        }

        if (!newFile->resize(newFile->pos())) {
            d->errorString = newFile->errorString();
            qCWarning(lcFileLocker) << "Failed to resize after reopen:" << d->errorString;
            return false;
        }
        
        if (!newFile->flush()) {
            d->errorString = newFile->errorString();
            qCWarning(lcFileLocker) << "Failed to flush during saveAs:" << d->errorString;
            newFile->close();
            return false;
        }
        
        // Successfully saved, now update our state
        
        // Remove old file from watcher
        d->watcher->removePath(d->filePath);
        
        // Replace current file with the new one
        d->file = std::move(newFile);
        d->filePath = path;
        
        // Add new file to watcher
        d->watcher->addPath(path);
        
        // Reset modification flag after successful saveAs
        if (d->isFileModifiedSinceLastSave) {
            d->isFileModifiedSinceLastSave = false;
            Q_EMIT fileModifiedSinceLastSaveChanged();
        }

        Q_EMIT pathChanged();
        Q_EMIT entryNameChanged();
        
        qCDebug(lcFileLocker) << "Successfully saved as" << data.size() << "bytes to file:" << path;
        return true;
    }

    void FileLocker::close() {
        Q_D(FileLocker);

        d->errorString.clear();

        d->file.reset();
        
        // Remove from watcher
        d->watcher->removePath(d->filePath);
        
        // Clear file path
        d->filePath.clear();
        
        // Reset modification flag
        if (d->isFileModifiedSinceLastSave) {
            d->isFileModifiedSinceLastSave = false;
            Q_EMIT fileModifiedSinceLastSaveChanged();
        }
        
        Q_EMIT pathChanged();
        Q_EMIT entryNameChanged();
        
        qCDebug(lcFileLocker) << "Closed file and cleared path";
    }

}

#include "moc_filelocker.cpp"