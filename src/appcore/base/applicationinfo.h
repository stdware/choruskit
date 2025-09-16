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

        enum SystemLocation {
            Binary,
            Library,
            Resources,
            AppData,
        };

        static QString systemLocation(SystemLocation location);

        enum ApplicationLocation {
            BuiltinResources,
            BuiltinPlugins,
            UserConfig,
            RuntimeData,
            TempData,
        };

        static QString applicationLocation(ApplicationLocation directory);

    public:
        static QString translate(const char *context, const char *sourceText,
                                 const char *disambiguation = nullptr, int n = -1,
                                 bool *ok = nullptr);

    private:
        ApplicationInfo() = delete;
    };

}

#endif // CHORUSKIT_APPLICATIONINFO_H
