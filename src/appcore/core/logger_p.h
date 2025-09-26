#ifndef CHORUSKIT_LOGGER_P_H
#define CHORUSKIT_LOGGER_P_H

#include <QMutex>
#include <QFile>
#include <QTextStream>

#include <CoreApi/logger.h>

namespace Core {

    class LoggerPrivate {
        Q_DECLARE_PUBLIC(Logger)
    public:
        Logger *q_ptr;

        qsizetype maxFileSize = 16 * 1024 * 1024; // 16 MiB in bytes
        qsizetype maxArchiveSize = 1024LL * 1024 * 1024; // 1 GiB in bytes
        int maxArchiveDays = 30;
        bool prettifiesConsoleOutput = true;
        Logger::MessageType consoleLogLevel = Logger::Info;
        Logger::MessageType fileLogLevel = Logger::Info;
        int compressLevel = 9;

        // Private implementation members
        mutable QRecursiveMutex mutex;
        QString currentLogFile;
        QFile *logFile = nullptr;
        QTextStream *logStream = nullptr;

        void rotateLogFile();
        void cleanupOldArchives() const;
        void archiveExistingLogFiles();
        void compressAndArchiveFile(const QString &filePath);
        void writeToConsole(Logger::MessageType type, const QString &category, const QString &message, const QDateTime &now) const;
        void writeToFile(Logger::MessageType type, const QString &category, const QString &message, const QDateTime &now);
    };

}

#endif // CHORUSKIT_LOGGER_P_H
