#ifndef CHORUSKIT_LOGGER_H
#define CHORUSKIT_LOGGER_H

#include <QObject>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class LoggerPrivate;

    class CKAPPCORE_EXPORT Logger : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(Logger)

        Q_PROPERTY(qsizetype maxFileSize READ maxFileSize WRITE setMaxFileSize NOTIFY maxFileSizeChanged)
        Q_PROPERTY(qsizetype maxArchiveSize READ maxArchiveSize WRITE setMaxArchiveSize NOTIFY maxArchiveSizeChanged)
        Q_PROPERTY(int maxArchiveDays READ maxArchiveDays WRITE setMaxArchiveDays NOTIFY maxArchiveDaysChanged)
        Q_PROPERTY(bool prettifiesConsoleOutput READ prettifiesConsoleOutput WRITE setPrettifiesConsoleOutput NOTIFY prettifiesConsoleOutputChanged)
        Q_PROPERTY(MessageType consoleLogLevel READ consoleLogLevel WRITE setConsoleLogLevel NOTIFY consoleLogLevelChanged)
        Q_PROPERTY(MessageType fileLogLevel READ fileLogLevel WRITE setFileLogLevel NOTIFY fileLogLevelChanged)

    public:
        explicit Logger(QObject *parent = nullptr);
        ~Logger() override;

        qsizetype maxFileSize() const;
        void setMaxFileSize(qsizetype maxFileSize);

        qsizetype maxArchiveSize() const;
        void setMaxArchiveSize(qsizetype maxArchiveSize);

        int maxArchiveDays() const;
        void setMaxArchiveDays(int maxArchiveDays);

        bool prettifiesConsoleOutput() const;
        void setPrettifiesConsoleOutput(bool prettifiesConsoleOutput);

        enum MessageType {
            Debug,
            Info,
            Warning,
            Critical,
            Fatal,
        };
        Q_ENUM(MessageType)

        MessageType consoleLogLevel() const;
        void setConsoleLogLevel(MessageType consoleLogLevel);

        MessageType fileLogLevel() const;
        void setFileLogLevel(MessageType fileLogLevel);

        void loadSettings();
        void saveSettings() const;

        static QString logsLocation();

        Q_INVOKABLE void log(MessageType type, const QString &category, const QString &message);

    Q_SIGNALS:
        void maxFileSizeChanged(qsizetype maxFileSize);
        void maxArchiveSizeChanged(qsizetype maxArchiveSize);
        void maxArchiveDaysChanged(int maxArchiveDays);
        void prettifiesConsoleOutputChanged(bool prettifiesConsoleOutput);
        void consoleLogLevelChanged(MessageType consoleLogLevel);
        void fileLogLevelChanged(MessageType fileLogLevel);

        void messageLogged(MessageType type, const QString &category, const QString &message);

    private:
        QScopedPointer<LoggerPrivate> d_ptr;
    };

}

#endif // CHORUSKIT_LOGGER_H
