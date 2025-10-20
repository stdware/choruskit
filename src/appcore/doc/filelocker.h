#ifndef CHORUSKIT_FILELOCKER_H
#define CHORUSKIT_FILELOCKER_H

#include <QObject>
#include <qqmlintegration.h>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class FileLockerPrivate;

    class CKAPPCORE_EXPORT FileLocker : public QObject {
        Q_OBJECT
        QML_ELEMENT
        Q_DECLARE_PRIVATE(FileLocker)
        
        Q_PROPERTY(QString path READ path NOTIFY pathChanged)
        Q_PROPERTY(bool readOnly READ isReadOnly NOTIFY readOnlyChanged)

    public:
        explicit FileLocker(QObject *parent = nullptr);
        ~FileLocker() override;

        QString path() const;
        bool isReadOnly() const;

        Q_INVOKABLE bool open(const QString &path);
        Q_INVOKABLE QByteArray readData();
        Q_INVOKABLE bool save(const QByteArray &data);
        Q_INVOKABLE bool saveAs(const QString &path, const QByteArray &data);
        Q_INVOKABLE void close();

    Q_SIGNALS:
        void pathChanged();
        void readOnlyChanged();

    private:
        QScopedPointer<FileLockerPrivate> d_ptr;
    };

}

#endif // CHORUSKIT_FILELOCKER_H