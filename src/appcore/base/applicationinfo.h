#ifndef CHORUSKIT_APPLICATIONINFO_H
#define CHORUSKIT_APPLICATIONINFO_H

#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtWidgets/QSplashScreen>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class CKAPPCORE_EXPORT ApplicationInfo {
    public:
        Q_DECL_CONSTEXPR static inline int unitDpi() {
#ifdef Q_OS_MACOS
            return 72;
#else
            return 96;
#endif
        }

        static bool isUserRoot();

        enum MessageBoxIcon {
            NoIcon,
            Information,
            Question,
            Warning,
            Critical,
        };

        static void messageBox(QObject *parent, MessageBoxIcon flag, const QString &title,
                               const QString &text);

        static QString defaultAppDataDir();

    public:
        static QString appDataDir();
        static void setAppDataDir(const QString &dir);

        static QString userDataDir();
        static void setUserDataDir(const QString &dir);

        static QString tempDir();
        void setTempDir(const QString &dir);

        static QString libDir();
        void setLibDir(const QString &dir);

        static QString shareDir();
        void setShareDir(const QString &dir);

        static QString appShareDir();
        void setAppShareDir(const QString &dir);

        static QString appPluginsDir();
        void setAppPluginsDir(const QString &dir);

    public:
        static QString configurationPath(QSettings::Scope scope = QSettings::UserScope);
        static QString configurationBasePrefix();

        static QString translate(const char *context, const char *sourceText,
                                 const char *disambiguation = nullptr, int n = -1,
                                 bool *ok = nullptr);

    private:
        ApplicationInfo() = delete;
    };

}

#endif // CHORUSKIT_APPLICATIONINFO_H