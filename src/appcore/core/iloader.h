#ifndef ILOADER_H
#define ILOADER_H

#include <QDateTime>
#include <QJsonObject>
#include <QSettings>

#include <CoreApi/objectpool.h>

namespace Core {

    class ILoaderPrivate;

    class CKAPPCORE_EXPORT ILoader : public ObjectPool {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ILoader);
    public:
        explicit ILoader(QObject *parent = nullptr);
        ~ILoader();

        static ILoader *instance();

        static QDateTime startTime();

    public:
        QString settingsPath(QSettings::Scope scope) const;
        void setSettingsPath(QSettings::Scope scope, const QString &path);

        void readSettings();
        void writeSettings() const;

        QJsonObject *settings(QSettings::Scope scope = QSettings::UserScope);
    };

}


#endif // ILOADER_H
