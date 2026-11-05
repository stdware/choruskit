#include "FileOpenEventSource.h"
#include "FileOpenEventSource_p.h"

#include <utility>

#include <QCoreApplication>
#include <QEvent>
#include <QFileOpenEvent>
#include <QUrl>

namespace Core {

    FileOpenEventSource::FileOpenEventSource(QObject *parent) : QObject(parent), d_ptr(new FileOpenEventSourcePrivate) {
        Q_D(FileOpenEventSource);
        d->q_ptr = this;
        Q_ASSERT(QCoreApplication::instance());
        QCoreApplication::instance()->installEventFilter(this);
    }

    FileOpenEventSource::~FileOpenEventSource() = default;

    QStringList FileOpenEventSource::takePendingFiles() {
        Q_D(FileOpenEventSource);
        return std::exchange(d->pendingFiles, {});
    }

    bool FileOpenEventSource::eventFilter(QObject *watched, QEvent *event) {
        if (watched == QCoreApplication::instance() && event->type() == QEvent::FileOpen) {
            const auto filePath = static_cast<QFileOpenEvent *>(event)->url().toLocalFile();
            if (!filePath.isEmpty()) {
                Q_D(FileOpenEventSource);
                d->pendingFiles.append(filePath);
                Q_EMIT filesAvailable();
                return true;
            }
        }
        return QObject::eventFilter(watched, event);
    }

}

#include "moc_FileOpenEventSource.cpp"
