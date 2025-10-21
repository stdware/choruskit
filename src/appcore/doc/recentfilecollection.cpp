#include "recentfilecollection.h"
#include "recentfilecollection_p.h"

#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QSettings>
#include <QStandardPaths>
#include <QLoggingCategory>
#include <QPixmap>

#include <CoreApi/applicationinfo.h>
#include <CoreApi/runtimeinterface.h>

namespace Core {

    Q_STATIC_LOGGING_CATEGORY(lcRecentFileCollection, "ck.recentfilecollection")

    void RecentFileCollectionPrivate::init() {
        // Initialize thumbnail directory
        QString dataDir = ApplicationInfo::applicationLocation(ApplicationInfo::RuntimeData);
        thumbnailDir = QDir(dataDir).absoluteFilePath("thumbnails");
        ensureThumbnailDir();
    }

    QString RecentFileCollectionPrivate::thumbnailPath(const QString &filePath) const {
        QString canonical = canonicalFilePath(filePath);
        if (canonical.isEmpty()) {
            return {};
        }
        
        // Use MD5 hash to generate filename
        QByteArray hash = QCryptographicHash::hash(canonical.toUtf8(), QCryptographicHash::Md5);
        QString fileName = QString::fromLatin1(hash.toHex()) + ".png";
        return QDir(thumbnailDir).absoluteFilePath(fileName);
    }

    QString RecentFileCollectionPrivate::canonicalFilePath(const QString &path) const {
        QFileInfo info(path);
        return info.canonicalFilePath();
    }

    void RecentFileCollectionPrivate::ensureThumbnailDir() {
        QDir dir(thumbnailDir);
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                qCWarning(lcRecentFileCollection) << "Failed to create thumbnail directory:" << thumbnailDir;
            } else {
                qCDebug(lcRecentFileCollection) << "Created thumbnail directory:" << thumbnailDir;
            }
        }
    }

    void RecentFileCollectionPrivate::cleanupThumbnail(const QString &filePath) {
        QString thumbPath = thumbnailPath(filePath);
        if (!thumbPath.isEmpty() && QFile::exists(thumbPath)) {
            if (QFile::remove(thumbPath)) {
                qCDebug(lcRecentFileCollection) << "Removed thumbnail:" << thumbPath;
            } else {
                qCWarning(lcRecentFileCollection) << "Failed to remove thumbnail:" << thumbPath;
            }
        }
    }

    RecentFileCollection::RecentFileCollection(QObject *parent)
        : QObject(parent), d_ptr(new RecentFileCollectionPrivate) {
        Q_D(RecentFileCollection);
        d->q_ptr = this;
        d->init();
        loadSettings();
    }

    RecentFileCollection::~RecentFileCollection() {
        saveSettings();
    }

    QStringList RecentFileCollection::recentFiles() const {
        Q_D(const RecentFileCollection);
        return d->recentFiles;
    }

    int RecentFileCollection::count() const {
        Q_D(const RecentFileCollection);
        return d->count;
    }

    void RecentFileCollection::setCount(int count) {
        Q_D(RecentFileCollection);
        if (d->count == count) {
            return;
        }
        
        qCInfo(lcRecentFileCollection) << "Setting count from" << d->count << "to" << count;
        
        // If new count is smaller than current list length, need to remove excess files and cleanup thumbnails
        if (count < d->recentFiles.size()) {
            for (int i = count; i < d->recentFiles.size(); ++i) {
                d->cleanupThumbnail(d->recentFiles.at(i));
            }
            d->recentFiles = d->recentFiles.mid(0, count);
            Q_EMIT recentFilesChanged();
        }
        
        d->count = count;
        Q_EMIT countChanged(count);
        saveSettings();
    }

    bool RecentFileCollection::addRecentFile(const QString &path, const QPixmap &thumbnail) {
        Q_D(RecentFileCollection);
        
        // Check if file exists
        QFileInfo fileInfo(path);
        if (!fileInfo.exists()) {
            qCWarning(lcRecentFileCollection) << "File does not exist:" << path;
            return false;
        }
        
        QString canonicalPath = d->canonicalFilePath(path);
        if (canonicalPath.isEmpty()) {
            qCWarning(lcRecentFileCollection) << "Failed to get canonical path for:" << path;
            return false;
        }
        
        qCInfo(lcRecentFileCollection) << "Adding recent file:" << canonicalPath;
        
        // If file already exists, remove it first
        int existingIndex = -1;
        for (int i = 0; i < d->recentFiles.size(); ++i) {
            if (d->canonicalFilePath(d->recentFiles.at(i)) == canonicalPath) {
                existingIndex = i;
                break;
            }
        }
        
        if (existingIndex >= 0) {
            d->recentFiles.removeAt(existingIndex);
            qCDebug(lcRecentFileCollection) << "Removed existing entry at index" << existingIndex;
        }
        
        // Add to the beginning of the list
        d->recentFiles.prepend(canonicalPath);
        
        // Save thumbnail
        if (!thumbnail.isNull()) {
            QString thumbPath = d->thumbnailPath(canonicalPath);
            if (!thumbPath.isEmpty()) {
                if (!thumbnail.save(thumbPath, "PNG")) {
                    qCWarning(lcRecentFileCollection) << "Failed to save thumbnail for:" << canonicalPath;
                } else {
                    qCDebug(lcRecentFileCollection) << "Saved thumbnail to:" << thumbPath;
                }
            }
        }
        
        // Check list length, remove files exceeding the limit
        while (d->recentFiles.size() > d->count) {
            QString removedFile = d->recentFiles.takeLast();
            d->cleanupThumbnail(removedFile);
            qCDebug(lcRecentFileCollection) << "Removed oldest file due to count limit:" << removedFile;
        }
        
        Q_EMIT recentFilesChanged();
        saveSettings();
        
        return true;
    }

    bool RecentFileCollection::removeRecentFile(const QString &path) {
        Q_D(RecentFileCollection);

        QString canonicalPath = d->canonicalFilePath(path);
        if (canonicalPath.isEmpty()) {
            // If unable to get canonical path, try direct matching
            canonicalPath = path;
        }

        int removeIndex = -1;
        for (int i = 0; i < d->recentFiles.size(); ++i) {
            QString itemCanonical = d->canonicalFilePath(d->recentFiles.at(i));
            if (itemCanonical.isEmpty()) {
                itemCanonical = d->recentFiles.at(i);
            }

            if (itemCanonical == canonicalPath) {
                removeIndex = i;
                break;
            }
        }

        if (removeIndex < 0) {
            qCWarning(lcRecentFileCollection) << "File not found in recent list:" << path;
            return false;
        }

        QString removedFile = d->recentFiles.takeAt(removeIndex);
        d->cleanupThumbnail(removedFile);

        qCInfo(lcRecentFileCollection) << "Removed recent file:" << removedFile;

        Q_EMIT recentFilesChanged();
        saveSettings();

        return true;
    }

    void RecentFileCollection::clearRecentFile() {
        Q_D(RecentFileCollection);
        for (auto file : d->recentFiles) {
            d->cleanupThumbnail(file);
        }
        d->recentFiles.clear();
        qCInfo(lcRecentFileCollection) << "Cleared recent files";
        Q_EMIT recentFilesChanged();
        saveSettings();
    }

    QPixmap RecentFileCollection::thumbnail(const QString &path) {
        Q_D(const RecentFileCollection);
        
        QString thumbPath = d->thumbnailPath(path);
        if (thumbPath.isEmpty()) {
            return {};
        }
        
        QPixmap pixmap;
        if (!pixmap.load(thumbPath)) {
            qCDebug(lcRecentFileCollection) << "Failed to load thumbnail:" << thumbPath;
            return {};
        }
        
        return pixmap;
    }

    QString RecentFileCollection::thumbnailPath(const QString &path) {
        Q_D(const RecentFileCollection);
        return d->thumbnailPath(path);
    }

    void RecentFileCollection::loadSettings() {
        Q_D(RecentFileCollection);
        
        qCDebug(lcRecentFileCollection) << "Loading settings";
        
        auto settings = RuntimeInterface::settings();
        if (!settings) {
            qCWarning(lcRecentFileCollection) << "No settings instance available";
            return;
        }
        
        settings->beginGroup(staticMetaObject.className());
        d->recentFiles = settings->value("recentFiles").toStringList();
        d->count = settings->value("count", 32).toInt();
        settings->endGroup();
        
        qCDebug(lcRecentFileCollection) << "Loaded" << d->recentFiles.size() << "recent files, count limit:" << d->count;
    }

    void RecentFileCollection::saveSettings() const {
        Q_D(const RecentFileCollection);
        
        qCDebug(lcRecentFileCollection) << "Saving settings";
        
        auto settings = RuntimeInterface::settings();
        if (!settings) {
            qCWarning(lcRecentFileCollection) << "No settings instance available";
            return;
        }
        
        settings->beginGroup(staticMetaObject.className());
        settings->setValue("recentFiles", d->recentFiles);
        settings->setValue("count", d->count);
        settings->endGroup();
        
        qCDebug(lcRecentFileCollection) << "Saved" << d->recentFiles.size() << "recent files, count limit:" << d->count;
    }

}

#include "moc_recentfilecollection.cpp"