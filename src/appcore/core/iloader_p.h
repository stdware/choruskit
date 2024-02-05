#ifndef ILOADER_P_H
#define ILOADER_P_H

#include <CoreApi/iloader.h>

namespace Core {

    class ILoaderPrivate : public QObject {
        Q_DECLARE_PUBLIC(ILoader)
    public:
        ILoaderPrivate();
        ~ILoaderPrivate();

        void init();

        static bool readJson(const QString &filename, QJsonObject *out);
        static bool writeJson(const QString &filename, const QJsonObject &in);

        ILoader *q_ptr;

        QString globalSettingsPath;
        QString settingsPath;

        mutable bool settingsUnread;
        mutable bool settingsNeedWrite;

        QJsonObject globalSettings;
        QJsonObject userSettings;
    };

}

#endif // ILOADER_P_H
