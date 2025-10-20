#ifndef CHORUSKIT_RECENTFILECOLLECTION_P_H
#define CHORUSKIT_RECENTFILECOLLECTION_P_H

#include "recentfilecollection.h"

#include <QDir>

namespace Core {

    class RecentFileCollectionPrivate {
        Q_DECLARE_PUBLIC(RecentFileCollection)
    public:
        RecentFileCollection *q_ptr;
        
        QStringList recentFiles;
        int count = 32; // Default to save 32 recent files
        
        QString thumbnailDir; // Thumbnail storage directory
        
        void init();
        QString thumbnailPath(const QString &filePath) const;
        QString canonicalFilePath(const QString &path) const;
        void ensureThumbnailDir();
        void cleanupThumbnail(const QString &filePath);
    };

}

#endif // CHORUSKIT_RECENTFILECOLLECTION_P_H