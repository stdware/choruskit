#include "applicationinfo.h"

#ifdef Q_OS_WINDOWS
#  include <ShlObj.h>
#  include <Windows.h>
#elif defined(Q_OS_MACOS)
#  include <CoreFoundation/CoreFoundation.h>
#endif

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QCoreApplication>
#include <QtCore/QCoreApplication>
#include <QtWidgets/QMessageBox>

#include <QtCore/private/qcoreapplication_p.h>

#ifdef Q_OS_MAC
#  define OS_LIBRARY_DIR "Frameworks"
#  define OS_SHARE_DIR   "Resources"
#else
#  define OS_LIBRARY_DIR "lib"
#  define OS_SHARE_DIR   "share"
#endif

namespace Core {

    /*!
        \class ApplicationInfo

        \brief The ApplicationInfo class provides information about the application.

        \note Call this class after the QCoreApplication has been created.
    */

    /*!
        \enum ApplicationInfo::MessageBoxIcon
        \brief Message level enumeration.

        \var ApplicationInfo::NoIcon
        \brief Normal level.
        \var ApplicationInfo::Information
        \brief Information level.
        \var ApplicationInfo::Question
        \brief Question level.
        \var ApplicationInfo::Warning
        \brief Warning level.
        \var ApplicationInfo::Critical
        \brief Error level.
    */

    /*!
        \enum ApplicationInfo::SystemLocation
        \brief Application special directories.

        \var ApplicationInfo::Binary
        \brief The global binary directory.
        \var ApplicationInfo::Library
        \brief The global library directory.
        \var ApplicationInfo::Resources
        \brief The global resources directory.
        \var ApplicationInfo::AppData
        \brief The global application data directory.
    */

    /*!
        \enum ApplicationInfo::ApplicationLocation
        \brief Application special directories.

        \var ApplicationInfo::BuiltinResources
        \brief The application builtin resources directory.
        \var ApplicationInfo::BuiltinPlugins
        \brief The application builtin plugins directory.
        \var ApplicationInfo::UserConfig
        \brief The application user config directory.
        \var ApplicationInfo::RuntimeData
        \brief The application persistent runtime data directory.
        \var ApplicationInfo::TempData
        \brief The application temporary data directory.
    */

    static QString getDefaultAppDataDir() {
        static const auto path = []() {
            QString path;
            QString slashName;
#ifdef Q_OS_WINDOWS
            path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#elif defined(Q_OS_MAC)
            path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config";
#else
            path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
#endif
            slashName = '/' + QCoreApplication::applicationName();
            if (path.endsWith(slashName)) {
                path.chop(slashName.size());
            }
            slashName = '/' + QCoreApplication::organizationName();
            if (path.endsWith(slashName)) {
                path.chop(slashName.size());
            }
            return path;
        }();
        return path;
    }

    static QString appUpperDir() {
        static QString dir = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/..");
        return dir;
    }

    struct ApplicationInfoData {
        QString appDataDir;
        QString userDataDir;
        QString tempDir;
        QString libDir;
        QString shareDir;

        QString appShareDir;
        QString appPluginsDir;

        ApplicationInfoData() {
            appDataDir = getDefaultAppDataDir() + "/" + QCoreApplication::organizationName() + "/" +
                         QCoreApplication::applicationName();
            userDataDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
                          "/" + QCoreApplication::organizationName() + "/" +
                          QCoreApplication::applicationName();
            tempDir = QDir::tempPath() + "/" + QCoreApplication::organizationName() + "/" +
                      QCoreApplication::applicationName();
            libDir = appUpperDir() + "/lib";
            shareDir = appUpperDir() + "/share";

            // Set default app share dir and app plugins dir
            appShareDir = shareDir
#ifndef Q_OS_MAC
                          + "/" + qApp->applicationName()
#endif
                ;

            appPluginsDir =
#ifdef Q_OS_MAC
                appUpperDir() + "/Plugins"
#else
                libDir + "/" + QCoreApplication::applicationName() + "/plugins"
#endif
                ;
        }
    };

    /*!
        \fn int ApplicationInfo::unitDpi()

        Returns the system unit dpi value.

        \li on Mac, returns 72
        \li on Windows/Linux, returns 96
    */

    /*!
        Returns \c true if running with Administrator/Root privilege.
    */
    bool ApplicationInfo::isUserRoot() {
#ifdef Q_OS_WINDOWS
        return ::IsUserAnAdmin();
#else
        return ::geteuid() == 0;
#endif
    }

#if defined(Q_OS_WINDOWS) || defined(Q_OS_MAC)

    static void osMessageBox(void *winHandle, ApplicationInfo::MessageBoxIcon flag,
                             const QString &title, const QString &text) {
#  ifdef Q_OS_WINDOWS
        int winFlag;
        switch (flag) {
            case ApplicationInfo::NoIcon:
                winFlag = MB_OK;
                break;
            case ApplicationInfo::Critical:
                winFlag = MB_ICONERROR;
                break;
            case ApplicationInfo::Warning:
                winFlag = MB_ICONWARNING;
                break;
            case ApplicationInfo::Question:
                winFlag = MB_ICONQUESTION;
                break;
            case ApplicationInfo::Information:
                winFlag = MB_ICONINFORMATION;
                break;
        };

        ::MessageBoxW(static_cast<HWND>(winHandle), text.toStdWString().data(),
                      title.toStdWString().data(),
                      MB_OK
#    ifdef QTMEDIATE_WIN32_MSGBOX_TOPMOST
                          | MB_TOPMOST
#    endif
                          | MB_SETFOREGROUND | winFlag);
#  else
        // https://web.archive.org/web/20111127025605/http://jorgearimany.blogspot.com/2010/05/messagebox-from-windows-to-mac.html
        CFOptionFlags result;
        int level = 0;
        switch (flag) {
            case ApplicationInfo::Critical:
                level = 2;
                break;
            case ApplicationInfo::Warning:
                level = 1;
                break;
            case ApplicationInfo::Question:
                level = 3;
                break;
            default:
                level = 0;
                break;
        };
        CFUserNotificationDisplayAlert(
            0,     // no timeout
            level, // change it depending message_type flags ( MB_ICONASTERISK.... etc.)
            NULL,  // icon url, use default, you can change it depending message_type flags
            NULL,  // not used
            NULL,  // localization of strings
            title.toCFString(), // header text
            text.toCFString(),  // message text
            NULL,               // default "ok" text in button
            NULL,               // alternate button title
            NULL,               // other button title, null--> no other button
            &result             // response flags
        );
#  endif
    }

#endif

    /*!
        Shows system message box if supported, otherwise shows Qt message box.
    */
    void ApplicationInfo::messageBox(QObject *parent, MessageBoxIcon flag, const QString &title,
                                     const QString &text) {
        QWidget *w = nullptr;
        if (parent && parent->isWidgetType()) {
            w = qobject_cast<QWidget *>(parent)->window();
        }

#if defined(Q_OS_WINDOWS)
        osMessageBox(w ? reinterpret_cast<HWND>(w->winId()) : nullptr, flag, title, text);
#elif defined(Q_OS_MAC)
        osMessageBox(nullptr, flag, title, text);
#else
        switch (flag) {
            case Critical:
                QMessageBox::critical(w, title, text);
                break;
            case Warning:
                QMessageBox::warning(w, title, text);
                break;
            case Question:
                QMessageBox::question(w, title, text);
                break;
            case Information:
                QMessageBox::information(w, title, text);
                break;
        };
#endif
    }

    /*!
        Returns the system location.

    */
    QString ApplicationInfo::systemLocation(SystemLocation location) {
        QString res;
        switch (location) {
            case Binary:
                res = QCoreApplication::applicationDirPath();
                break;
            case Library:
                res = appUpperDir() + "/" OS_LIBRARY_DIR;
                break;
            case Resources:
                res = appUpperDir() + "/" OS_SHARE_DIR;
                break;
            case AppData:
                res = getDefaultAppDataDir();
                break;
            default:
                break;
        }
        return res;
    }

    /*!
        Returns the application location.
    */
    QString ApplicationInfo::applicationLocation(ApplicationLocation location) {
        QString res;
        switch (location) {
            case BuiltinResources:
                res = systemLocation(Resources)
#ifndef Q_OS_MAC
                      + "/" + QCoreApplication::applicationName()
#endif
                    ;
                break;
            case BuiltinPlugins:
#ifdef Q_OS_MAC
                res = appUpperDir() + "/Plugins";
#else
                res = systemLocation(Library) + "/" + QCoreApplication::applicationName() +
                      "/plugins";
#endif
                break;
            case UserConfig:
                res = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" +
                      QCoreApplication::organizationName() + "/" +
                      QCoreApplication::applicationName();
                break;
            case RuntimeData:
                res = systemLocation(AppData) + "/" + QCoreApplication::organizationName() + "/" +
                      QCoreApplication::applicationName();
                break;
            case TempData:
                res = QDir::tempPath() + "/" + QCoreApplication::organizationName() + "/" +
                      QCoreApplication::applicationName();
                break;
            default:
                break;
        }
        return res;
    }

    static void replacePercentN(QString *result, int n) {
        if (n >= 0) {
            int percentPos = 0;
            int len = 0;
            while ((percentPos = result->indexOf(QLatin1Char('%'), percentPos + len)) != -1) {
                len = 1;
                if (percentPos + len == result->length())
                    break;
                QString fmt;
                if (result->at(percentPos + len) == QLatin1Char('L')) {
                    ++len;
                    if (percentPos + len == result->length())
                        break;
                    fmt = QLatin1String("%L1");
                } else {
                    fmt = QLatin1String("%1");
                }
                if (result->at(percentPos + len) == QLatin1Char('n')) {
                    fmt = fmt.arg(n);
                    ++len;
                    result->replace(percentPos, len, fmt);
                    len = fmt.length();
                }
            }
        }
    }

    static QString tryTranslate(const char *context, const char *sourceText,
                                const char *disambiguation, int n, bool *ok) {
        if (ok)
            *ok = false;

        QString result;
        if (!sourceText) {
            return result;
        }

        class HackedApplication : public QCoreApplication {
        public:
            inline QCoreApplicationPrivate *d_func() {
                return static_cast<QCoreApplicationPrivate *>(d_ptr.data());
            }
        };

        auto self = QCoreApplication::instance();
        if (self) {
            QCoreApplicationPrivate *d = static_cast<HackedApplication *>(self)->d_func();
            QReadLocker locker(&d->translateMutex);
            if (!d->translators.isEmpty()) {
                QList<QTranslator *>::ConstIterator it;
                QTranslator *translationFile;
                for (it = d->translators.constBegin(); it != d->translators.constEnd(); ++it) {
                    translationFile = *it;
                    result = translationFile->translate(context, sourceText, disambiguation, n);
                    if (!result.isNull())
                        break;
                }
            }
        }

        if (result.isNull()) {
            result = QString::fromUtf8(sourceText);
        } else if (ok) {
            *ok = true;
        }
        replacePercentN(&result, n);
        return result;
    }

    /*!
        Returns the translation text for \a sourceText, along with the success flag.
    */
    QString ApplicationInfo::translate(const char *context, const char *sourceText,
                                       const char *disambiguation, int n, bool *ok) {
        return tryTranslate(context, sourceText, disambiguation, n, ok);
    }

    /*!
        Returns application data directory.

        \li On Mac/Linux, the default path is <tt>\%HOME\%/.config/\%ORG\%/\%AppName\%</tt>
        \li On Windows, the default path is
       <tt>\%UserProfile\%/AppData/Local/\%ORG\%/\%AppName\%</tt>
     */

    /*!
        Returns user data directory.

        \li On Mac/Linux, the default path is <tt>\%HOME\%/Documents/\%ORG\%/\%AppName\%</tt>
        \li On Windows, the default path is <tt>\%UserProfile\%/Documents/\%ORG\%/\%AppName\%</tt>
     */

    /*!
        Returns the application temporary directory.

        \li On Mac/Linux, the default path is <tt>\%TMPDIR\%</tt>
        \li On Windows, the default path is <tt>\%TEMP\%</tt>
    */

    /*!
        Returns the library directory.

        \li On Mac/Linux, the default path is <tt>\%AppPath\%/../Frameworks</tt>
        \li On Windows, the default path is <tt>\%AppPath\%/../lib</tt>
    */

    /*!
        Returns the share directory.

        \li On Mac, the default path is <tt>\%AppPath\%/../Resources</tt>
        \li On Windows/Linux, the default path is <tt>\%AppPath\%/../share</tt>
    */

    /*!
        Returns the application's share directory.

        \li On Mac, the default path is <tt>\%ShareDir\%</tt>
        \li On Windows/Linux, the default path is <tt>\%ShareDir\%/\%AppName\%</tt>
    */

    /*!
        Returns the application's plugins directory.

        \li On Mac, the default path is <tt>\%AppPath\%/../Plugins</tt>
        \li On Windows/Linux, the default path is <tt>\%LibDir\%/\%AppName\%/plugins</tt>
    */

}
