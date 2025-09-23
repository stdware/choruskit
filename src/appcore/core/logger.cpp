#include "logger.h"
#include "logger_p.h"

#include <QDateTime>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>
#include <QMutexLocker>
#include <QDebug>
#include <QFileInfo>
#include <QTimeZone>
#include <cstdio>

#include <CoreApi/runtimeinterface.h>
#include <CoreApi/applicationinfo.h>

namespace Core {

    static QString typeToString(Logger::MessageType type) {
        switch (type) {
            case Logger::Debug:
                return QStringLiteral("DEBUG");
            case Logger::Info:
                return QStringLiteral("INFO");
            case Logger::Warning:
                return QStringLiteral("WARNING");
            case Logger::Critical:
                return QStringLiteral("CRITICAL");
            case Logger::Fatal:
                return QStringLiteral("FATAL");
        }
        return {};
    }

    static QString typeToSGRCode(Logger::MessageType type) {
        switch (type) {
            case Logger::Debug:
                return QStringLiteral("36");
            case Logger::Info:
                return QStringLiteral("0");
            case Logger::Warning:
                return QStringLiteral("33");
            case Logger::Critical:
            case Logger::Fatal:
                return QStringLiteral("31");
        }
        return {};
    }

    static QString colorizeText(const QString &text, const QString &sgrCode) {
        return QStringLiteral("\033[0m\033[%1m%2\033[0m").arg(sgrCode, text);
    }

    static QString formatConsoleOutput(Logger::MessageType type, const QString &category, const QString &message, const QDateTime &now, bool prettifiesConsoleOutput) {
        static auto formatString = QStringLiteral("[%1 %2] [%3/%4]: %5");
        auto offset = now.timeZone().offsetFromUtc(now);
        auto hours = qAbs(offset) / 3600;
        auto minutes = qAbs(offset) % 3600 / 60;
        auto sign = offset >= 0 ? '+' : '-';
        auto offsetText = QStringLiteral("%1%2:%3")
            .arg(sign)
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'));

        auto text = formatString.arg(now.toString(Qt::ISODateWithMs), offsetText, category, typeToString(type), message);
        return prettifiesConsoleOutput ? colorizeText(text, typeToSGRCode(type)) : text;
    }

    static QString formatFileOutput(Logger::MessageType type, const QString &category, const QString &message, const QDateTime &now) {
        static auto formatString = QStringLiteral("[%1 %2] [%3/%4]: %5");
        auto offset = now.timeZone().offsetFromUtc(now);
        auto hours = qAbs(offset) / 3600;
        auto minutes = qAbs(offset) % 3600 / 60;
        auto sign = offset >= 0 ? '+' : '-';
        auto offsetText = QStringLiteral("%1%2:%3")
            .arg(sign)
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'));

        auto text = formatString.arg(now.toString(Qt::ISODateWithMs), offsetText, category, typeToString(type), message);
        return text;
    }

    static void ensureLogDirectoryExists() {
        const QString logsDir = Logger::logsLocation();
        QDir dir;
        if (!dir.exists(logsDir)) {
            dir.mkpath(logsDir);
        }
    }

    void LoggerPrivate::rotateLogFile() {
        // Close existing file
        if (logStream) {
            logStream->flush();
            delete logStream;
            logStream = nullptr;
        }
        if (logFile) {
            const QString oldFile = logFile->fileName();
            logFile->close();
            delete logFile;
            logFile = nullptr;

            // Archive the old file if it exists and has content
            if (QFileInfo(oldFile).size() > 0) {
                compressAndArchiveFile(oldFile);
            }
        }

        // Create new log file
        currentLogFile = generateLogFileName();
        logFile = new QFile(currentLogFile);
        if (logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            logStream = new QTextStream(logFile);
            logStream->setEncoding(QStringConverter::Utf8);

            // Cleanup old archives when rotating log file
            cleanupOldArchives();
        } else {
            outputCriticalToConsoleOnly(QStringLiteral("Failed to open log file: %1").arg(currentLogFile));
            delete logFile;
            logFile = nullptr;
        }
    }

    void LoggerPrivate::cleanupOldArchives() const {
        const QString logsDir = Logger::logsLocation();
        QDir dir(logsDir);

        const QStringList filters{QStringLiteral("*.log.gz")};
        const auto archiveFiles = dir.entryInfoList(filters, QDir::Files, QDir::Time);

        qint64 totalSize = 0;
        const auto cutoffDate = QDateTime::currentDateTime().addDays(-maxArchiveDays);

        QStringList filesToDelete;

        for (const auto &fileInfo : archiveFiles) {
            // Delete files older than maxArchiveDays
            if (fileInfo.lastModified() < cutoffDate) {
                filesToDelete << fileInfo.absoluteFilePath();
                continue;
            }

            totalSize += fileInfo.size();

            // If total archive size exceeds limit, mark older files for deletion
            if (totalSize > maxArchiveSize) {
                filesToDelete << fileInfo.absoluteFilePath();
            }
        }

        // Delete marked files
        for (const QString &filePath : filesToDelete) {
            QFile::remove(filePath);
        }
    }

    void LoggerPrivate::archiveExistingLogFiles() const {
        const QString logsDir = Logger::logsLocation();
        QDir dir(logsDir);

        const QStringList filters{QStringLiteral("*.log")};
        const auto logFiles = dir.entryInfoList(filters, QDir::Files, QDir::Time);

        // Archive all existing uncompressed log files
        for (const auto &fileInfo : logFiles) {
            const QString filePath = fileInfo.absoluteFilePath();
            
            // Only compress files that have content and don't already have a compressed version
            if (fileInfo.size() > 0) {
                const QString archiveFileName = generateArchiveFileName(filePath);
                if (!QFile::exists(archiveFileName)) {
                    compressAndArchiveFile(filePath);
                }
            } else {
                // Remove empty log files
                QFile::remove(filePath);
            }
        }
    }

    QString LoggerPrivate::generateLogFileName() {
        const QString logsDir = Logger::logsLocation();
        const auto now = QDateTime::currentDateTimeUtc();
        const QString timestamp = now.toString(Qt::ISODate).replace(QLatin1Char(':'), QLatin1Char('-'));

        QString baseName = QStringLiteral("%1/%2").arg(logsDir, timestamp);

        int counter = 0;
        QString fileName;
        do {
            if (counter == 0) {
                fileName = QStringLiteral("%1.log").arg(baseName);
            } else {
                fileName = QStringLiteral("%1_%2.log").arg(baseName, QString::number(counter));
            }
            ++counter;
        } while (QFile::exists(fileName) && counter < 1000); // Safety limit

        return fileName;
    }

    QString LoggerPrivate::generateArchiveFileName(const QString &originalFile) {
        return originalFile + QStringLiteral(".gz");
    }

    void LoggerPrivate::compressAndArchiveFile(const QString &filePath) const {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            outputCriticalToConsoleOnly(QStringLiteral("Failed to open file for compression: %1").arg(filePath));
            return;
        }

        const QByteArray data = file.readAll();
        file.close();

        // Simple compression using qCompress (zlib format)
        // FIXME should use gzip compress
        // const QByteArray compressed = qCompress(data, 9); // Maximum compression level
        const QByteArray compressed = data;

        const QString archiveFileName = generateArchiveFileName(filePath);
        QFile archiveFile(archiveFileName);
        if (archiveFile.open(QIODevice::WriteOnly)) {
            archiveFile.write(compressed);
            archiveFile.close();

            // Remove original file after successful compression
            QFile::remove(filePath);
        } else {
            outputCriticalToConsoleOnly(QStringLiteral("Failed to create archive file: %1").arg(archiveFileName));
        }
    }

    void LoggerPrivate::writeToConsole(Logger::MessageType type, const QString &category, const QString &message, const QDateTime &now) const {
        const QString consoleOutput = formatConsoleOutput(type, category, message, now, prettifiesConsoleOutput);
        // Output to stderr for better compatibility with redirection
        FILE *stderrFile = stderr;
        QTextStream stderrStream(stderrFile, QIODevice::WriteOnly);
        stderrStream.setEncoding(QStringConverter::System);
        stderrStream << consoleOutput << Qt::endl;
        stderrStream.flush();
    }

    void LoggerPrivate::writeToFile(Logger::MessageType type, const QString &category, const QString &message, const QDateTime &now) {
        // Ensure log file is ready
        if (!logFile || QFileInfo(currentLogFile).fileName().mid(0, 10) != now.toUTC().toString(Qt::ISODate).mid(0, 10)) {
            rotateLogFile();
        }

        // Check if current log file exceeds size limit
        if (logFile && logFile->size() > maxFileSize) {
            rotateLogFile();
        }

        if (logStream) {
            const QString fileOutput = formatFileOutput(type, category, message, now);
            *logStream << fileOutput << Qt::endl;
            logStream->flush();
        } else {
            // If file logging fails, output critical message to console only
            outputCriticalToConsoleOnly(QStringLiteral("Failed to write log message to file - file logging unavailable"));
        }
    }

    void LoggerPrivate::outputCriticalToConsoleOnly(const QString &message) const {
        // Direct console output without going through the log system to avoid recursion
        const auto now = QDateTime::currentDateTime();
        const QString criticalOutput = formatConsoleOutput(Logger::Critical, QStringLiteral("ck.logger"), message, now, prettifiesConsoleOutput);

        FILE *stderrFile = stderr;
        QTextStream stderrStream(stderrFile, QIODevice::WriteOnly);
        stderrStream << criticalOutput << Qt::endl;
        stderrStream.flush();
    }

    Logger::Logger(QObject *parent) : QObject(parent), d_ptr(new LoggerPrivate) {
        Q_D(Logger);
        d->q_ptr = this;
        ensureLogDirectoryExists();
        d->archiveExistingLogFiles();
        loadSettings();
    }

    Logger::~Logger() {
        Q_D(Logger);
        if (d->logStream) {
            d->logStream->flush();
            delete d->logStream;
        }
        if (d->logFile) {
            d->logFile->close();
            delete d->logFile;
        }
    }

    qsizetype Logger::maxFileSize() const {
        Q_D(const Logger);
        return d->maxFileSize;
    }

    void Logger::setMaxFileSize(qsizetype maxFileSize) {
        Q_D(Logger);
        if (d->maxFileSize == maxFileSize)
            return;
        
        d->maxFileSize = maxFileSize;
        Q_EMIT maxFileSizeChanged(maxFileSize);
    }

    qsizetype Logger::maxArchiveSize() const {
        Q_D(const Logger);
        return d->maxArchiveSize;
    }

    void Logger::setMaxArchiveSize(qsizetype maxArchiveSize) {
        Q_D(Logger);
        if (d->maxArchiveSize == maxArchiveSize)
            return;
        
        d->maxArchiveSize = maxArchiveSize;
        Q_EMIT maxArchiveSizeChanged(maxArchiveSize);
    }

    int Logger::maxArchiveDays() const {
        Q_D(const Logger);
        return d->maxArchiveDays;
    }

    void Logger::setMaxArchiveDays(int maxArchiveDays) {
        Q_D(Logger);
        if (d->maxArchiveDays == maxArchiveDays)
            return;
        
        d->maxArchiveDays = maxArchiveDays;
        Q_EMIT maxArchiveDaysChanged(maxArchiveDays);
    }

    bool Logger::prettifiesConsoleOutput() const {
        Q_D(const Logger);
        return d->prettifiesConsoleOutput;
    }

    void Logger::setPrettifiesConsoleOutput(bool prettifiesConsoleOutput) {
        Q_D(Logger);
        if (d->prettifiesConsoleOutput == prettifiesConsoleOutput)
            return;
        
        d->prettifiesConsoleOutput = prettifiesConsoleOutput;
        Q_EMIT prettifiesConsoleOutputChanged(prettifiesConsoleOutput);
    }

    Logger::MessageType Logger::consoleLogLevel() const {
        Q_D(const Logger);
        return d->consoleLogLevel;
    }

    void Logger::setConsoleLogLevel(MessageType consoleLogLevel) {
        Q_D(Logger);
        if (d->consoleLogLevel == consoleLogLevel)
            return;
        
        d->consoleLogLevel = consoleLogLevel;
        Q_EMIT consoleLogLevelChanged(consoleLogLevel);
    }

    Logger::MessageType Logger::fileLogLevel() const {
        Q_D(const Logger);
        return d->fileLogLevel;
    }

    void Logger::setFileLogLevel(MessageType fileLogLevel) {
        Q_D(Logger);
        if (d->fileLogLevel == fileLogLevel)
            return;
        
        d->fileLogLevel = fileLogLevel;
        Q_EMIT fileLogLevelChanged(fileLogLevel);
    }

    void Logger::loadSettings() {
        auto settings = RuntimeInterface::settings();
        settings->beginGroup(staticMetaObject.className());
        
        setMaxFileSize(settings->value("maxFileSize", 16 * 1024 * 1024).value<qsizetype>());
        setMaxArchiveSize(settings->value("maxArchiveSize", 1024LL * 1024 * 1024).value<qsizetype>());
        setMaxArchiveDays(settings->value("maxArchiveDays", 30).toInt());
        setPrettifiesConsoleOutput(settings->value("prettifiesConsoleOutput", true).toBool());
        setConsoleLogLevel(static_cast<MessageType>(settings->value("consoleLogLevel", static_cast<int>(Info)).toInt()));
        setFileLogLevel(static_cast<MessageType>(settings->value("fileLogLevel", static_cast<int>(Info)).toInt()));
        
        settings->endGroup();
    }

    void Logger::saveSettings() const {
        Q_D(const Logger);
        auto settings = RuntimeInterface::settings();
        settings->beginGroup(staticMetaObject.className());

        settings->setValue("maxFileSize", d->maxFileSize);
        settings->setValue("maxArchiveSize", d->maxArchiveSize);
        settings->setValue("maxArchiveDays", d->maxArchiveDays);
        settings->setValue("prettifiesConsoleOutput", d->prettifiesConsoleOutput);
        settings->setValue("consoleLogLevel", static_cast<int>(d->consoleLogLevel));
        settings->setValue("fileLogLevel", static_cast<int>(d->fileLogLevel));

        settings->endGroup();
    }

    QString Logger::logsLocation() {
        return ApplicationInfo::applicationLocation(ApplicationInfo::RuntimeData) + QStringLiteral("/logs");
    }

    void Logger::log(MessageType type, const QString &category, const QString &message) {
        Q_D(Logger);
        const auto now = QDateTime::currentDateTime();

        {
            QMutexLocker locker(&d->mutex);

            // Console output
            if (type >= d->consoleLogLevel) {
                d->writeToConsole(type, category, message, now);
            }

            // File output
            if (type >= d->fileLogLevel) {
                d->writeToFile(type, category, message, now);
            }

        }

        if (type >= d->consoleLogLevel) {
            Q_EMIT messageLogged(type, category, message);
        }
    }

}
