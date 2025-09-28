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
#include <QLoggingCategory>
#include <QTimeZone>
#ifdef Q_OS_WIN // TODO I'm not sure
#   include <QtZlib/zlib.h>
#else
#   include <zlib.h>
#endif

#include <CoreApi/runtimeinterface.h>
#include <CoreApi/applicationinfo.h>

namespace Core {

    static const QString lcTextLogger = "ck.logger";

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
        static auto formatString = QStringLiteral("[%1%2] [%3] [%4]: %5");
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
        static auto formatString = QStringLiteral("[%1 %2] [%3] [%4]: %5");
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

    static QString generateLogFileName() {
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

    static QByteArray compressData(const QByteArray &input, int compressLevel) {
        z_stream strm;
        std::memset(&strm, 0, sizeof(strm));

        int windowBits = MAX_WBITS + 16;
        int memLevel = 8; // 推荐值
        int strategy = Z_DEFAULT_STRATEGY;

        int ret = deflateInit2(&strm, compressLevel, Z_DEFLATED, windowBits, memLevel, strategy);
        if (ret != Z_OK) {
            return {};
        }

        const int CHUNK = 16384;
        QByteArray output;
        output.reserve(input.size() / 2 + CHUNK);

        strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
        strm.avail_in = static_cast<uInt>(input.size());

        do {
            unsigned char outbuf[CHUNK];
            strm.next_out = outbuf;
            strm.avail_out = CHUNK;

            ret = deflate(&strm, strm.avail_in ? Z_NO_FLUSH : Z_FINISH);
            if (ret == Z_STREAM_ERROR) {
                deflateEnd(&strm);
                return {};
            }

            int have = CHUNK - strm.avail_out;
            if (have > 0) {
                output.append(reinterpret_cast<char*>(outbuf), have);
            }
        } while (ret != Z_STREAM_END);

        deflateEnd(&strm);

        return output;
    }

    static QString generateArchiveFileName(const QString &originalFile) {
        return originalFile + QStringLiteral(".gz");
    }

    void LoggerPrivate::rotateLogFile() {
        Q_Q(Logger);
        q->log(Logger::Debug, lcTextLogger, "Rotating log file", true);
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
            q->log(Logger::Critical, lcTextLogger, QStringLiteral("Failed to open log file: %1").arg(currentLogFile));
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

    void LoggerPrivate::archiveExistingLogFiles() {
        Q_Q(Logger);

        const QString logsDir = Logger::logsLocation();
        QDir dir(logsDir);

        const QStringList filters{QStringLiteral("*.log")};
        const auto logFiles = dir.entryInfoList(filters, QDir::Files, QDir::Time);

        // Archive all existing uncompressed log files
        for (const auto &fileInfo : logFiles) {
            const QString filePath = fileInfo.absoluteFilePath();

            q->log(Logger::Debug, lcTextLogger, "Archiving existing log file: " + filePath);
            
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

    void LoggerPrivate::compressAndArchiveFile(const QString &filePath) {
        Q_Q(Logger);
        q->log(Logger::Debug, lcTextLogger, "Compressing file: " + filePath);
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            q->log(Logger::Critical, lcTextLogger, QStringLiteral("Failed to open file for compression: %1").arg(filePath), true);
            return;
        }

        const QByteArray data = file.readAll();
        file.close();

        // Simple compression using qCompress (zlib format)
        const QByteArray compressed = compressData(data, compressLevel);

        if (compressed.isEmpty()) {
            q->log(Logger::Critical, lcTextLogger, QStringLiteral("Failed to compress file: %1").arg(filePath), true);
            return;
        }

        const QString archiveFileName = generateArchiveFileName(filePath);
        QFile archiveFile(archiveFileName);
        if (archiveFile.open(QIODevice::WriteOnly)) {
            archiveFile.write(compressed);
            archiveFile.close();

            // Remove original file after successful compression
            QFile::remove(filePath);
        } else {
            q->log(Logger::Critical, lcTextLogger, QStringLiteral("Failed to create archive file: %1").arg(archiveFileName), true);
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
        Q_Q(Logger);
        // Ensure log file is ready
        if (!logFile) {
            q->log(Logger::Debug, lcTextLogger, "No log file", true);
            rotateLogFile();
        }

        // Check if current log file exceeds size limit
        if (logFile && logFile->size() > maxFileSize) {
            q->log(Logger::Debug, lcTextLogger, "Log file exceeds size limit", true);
            rotateLogFile();
        }

        if (logStream) {
            const QString fileOutput = formatFileOutput(type, category, message, now);
            *logStream << fileOutput << Qt::endl;
            logStream->flush();
        } else {
            // If file logging fails, output critical message to console only
            q->log(Logger::Critical, lcTextLogger, QStringLiteral("Failed to write log message to file - file logging unavailable"), true);
        }
    }

    Logger::Logger(QObject *parent) : QObject(parent), d_ptr(new LoggerPrivate) {
        Q_D(Logger);
        d->q_ptr = this;
        ensureLogDirectoryExists();
        loadSettings();
        d->archiveExistingLogFiles();
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

    int Logger::compressLevel() const {
        Q_D(const Logger);
        return d->compressLevel;
    }

    void Logger::setCompressLevel(int compressLevel) {
        Q_D(Logger);
        if (d->compressLevel == compressLevel)
            return;
        d->compressLevel = compressLevel;
        Q_EMIT compressLevelChanged(compressLevel);
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
        setCompressLevel(settings->value("compressLevel", 9).toInt());
        
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
        settings->setValue("compressLevel", d->compressLevel);

        settings->endGroup();
    }

    QString Logger::logsLocation() {
        return QDir::toNativeSeparators(ApplicationInfo::applicationLocation(ApplicationInfo::RuntimeData) + QStringLiteral("/logs"));
    }

    void Logger::log(MessageType type, const QString &category, const QString &message, bool onlyConsole) {
        Q_D(Logger);
        const auto now = QDateTime::currentDateTime();

        {
            QMutexLocker locker(&d->mutex);

            // Console output
            if (
#ifdef QT_DEBUG
                true // ignore log level in debug mode
#else
                type >= d->consoleLogLevel
#endif
            ) {
                d->writeToConsole(type, category, message, now);
            }

            // File output
            if (!onlyConsole && type >= d->fileLogLevel) {
                d->writeToFile(type, category, message, now);
            }
        }

        Q_EMIT messageLogged(type, category, message);
    }

}
