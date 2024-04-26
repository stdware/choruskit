#ifndef WINDOWSTATECACHE_H
#define WINDOWSTATECACHE_H

#include <QWidget>
#include <QSplitter>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class CKAPPCORE_EXPORT WindowStateCache {
    public:
        WindowStateCache();

        static WindowStateCache *instance();

    public:
        void loadWindowGeometry(const QString &id, QWidget *w, const QSize &fallback = {}) const;
        void saveWindowGeometry(const QString &id, QWidget *w);

        void loadSplitterSizes(const QString &id, QSplitter *s,
                               const QList<int> &fallback = {}) const;
        void saveSplitterSizes(const QString &id, QSplitter *s);
    };

}

#endif // WINDOWSTATECACHE_H
