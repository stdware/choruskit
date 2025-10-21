#ifndef CHORUSKIT_RECENTFILECOLLECTION_H
#define CHORUSKIT_RECENTFILECOLLECTION_H

#include <QObject>
#include <qqmlintegration.h>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class RecentFileCollectionPrivate;

    class CKAPPCORE_EXPORT RecentFileCollection : public QObject {
        Q_OBJECT
        QML_ELEMENT
        QML_UNCREATABLE("")
        Q_DECLARE_PRIVATE(RecentFileCollection)
        
        Q_PROPERTY(QStringList recentFiles READ recentFiles NOTIFY recentFilesChanged)
        Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)

    public:
        explicit RecentFileCollection(QObject *parent = nullptr);
        ~RecentFileCollection() override;

        QStringList recentFiles() const;
        
        int count() const;
        void setCount(int count);

        Q_INVOKABLE bool addRecentFile(const QString &path, const QPixmap &thumbnail);
        Q_INVOKABLE bool removeRecentFile(const QString &path);
        Q_INVOKABLE void clearRecentFile();
        Q_INVOKABLE QPixmap thumbnail(const QString &path);
        Q_INVOKABLE QString thumbnailPath(const QString &path);

        void loadSettings();
        void saveSettings() const;

    Q_SIGNALS:
        void recentFilesChanged();
        void countChanged(int count);

    private:
        QScopedPointer<RecentFileCollectionPrivate> d_ptr;
    };

}

#endif // CHORUSKIT_RECENTFILECOLLECTION_H