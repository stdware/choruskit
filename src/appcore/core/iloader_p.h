#ifndef ILOADER_P_H
#define ILOADER_P_H

#include <CoreApi/iloader.h>
#include <CoreApi/private/objectpool_p.h>

namespace Core {

    class ILoaderPrivate : public ObjectPoolPrivate {
        Q_DECLARE_PUBLIC(ILoader)
    public:
        ILoaderPrivate();
        ~ILoaderPrivate();

        void init();

        static bool readJson(const QString &filename, QJsonObject *out);
        static bool writeJson(const QString &filename, const QJsonObject &in);

        QString globalSettingsPath;
        QString settingsPath;

        mutable bool settingsUnread;
        mutable bool settingsNeedWrite;

        QJsonObject globalSettings;
        QJsonObject userSettings;

        CKAPPCORE_EXPORT static void *&quickData(int index); // index <= 512
    };

}

#endif // ILOADER_P_H
