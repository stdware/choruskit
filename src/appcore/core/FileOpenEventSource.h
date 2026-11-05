#ifndef CHORUSKIT_FILEOPENEVENTSOURCE_H
#define CHORUSKIT_FILEOPENEVENTSOURCE_H

#include <QObject>
#include <QScopedPointer>
#include <QStringList>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class FileOpenEventSourcePrivate;

    class CKAPPCORE_EXPORT FileOpenEventSource : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(FileOpenEventSource)
    public:
        explicit FileOpenEventSource(QObject *parent = nullptr);
        ~FileOpenEventSource() override;

        QStringList takePendingFiles();

    Q_SIGNALS:
        void filesAvailable();

    protected:
        bool eventFilter(QObject *watched, QEvent *event) override;

    private:
        QScopedPointer<FileOpenEventSourcePrivate> d_ptr;
    };

}

#endif // CHORUSKIT_FILEOPENEVENTSOURCE_H
