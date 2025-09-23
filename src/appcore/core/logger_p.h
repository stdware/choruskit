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

        // Private implementation members
        mutable QMutex mutex;
        QString currentLogFile;
        QFile *logFile = nullptr;
        QTextStream *logStream = nullptr;

        void rotateLogFile();
        void cleanupOldArchives() const;
        void archiveExistingLogFiles() const;
        static QString generateLogFileName();
        static QString generateArchiveFileName(const QString &originalFile);
        void compressAndArchiveFile(const QString &filePath) const;
        void writeToConsole(Logger::MessageType type, const QString &category, const QString &message, const QDateTime &now) const;
        void writeToFile(Logger::MessageType type, const QString &category, const QString &message, const QDateTime &now);
        void outputCriticalToConsoleOnly(const QString &message) const;
    };

}

#endif // CHORUSKIT_LOGGER_P_H
