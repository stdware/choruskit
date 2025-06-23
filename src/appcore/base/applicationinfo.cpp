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
#include <QtWidgets/QMessageBox>

namespace Core {

    /*!
        \class ApplicationInfo

        \brief The ApplicationInfo class provides information about the application.

        \note Call this class after the QCoreApplication has been created.
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

    Q_GLOBAL_STATIC(ApplicationInfoData, appInfo);

    /*!
        \fn int  ApplicationInfo::unitDpi()

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
        Returns the standard AppData location.

        \li On Windows, returns <tt>\%UserProfile\%/AppData</tt>
        \li On Mac/Linux, returns <tt>\%HOME\%/.config</tt>
    */
    QString ApplicationInfo::defaultAppDataDir() {
        return getDefaultAppDataDir();
    }

    /*!
        Returns application data directory.

        \li On Mac/Linux, the default path is <tt>\%HOME\%/.config/\%ORG\%/\%AppName\%</tt>
        \li On Windows, the default path is
       <tt>\%UserProfile\%/AppData/Local/\%ORG\%/\%AppName\%</tt>
     */
    QString ApplicationInfo::appDataDir() {
        return appInfo->appDataDir;
    }

    /*!
        Sets the application data directory.
    */
    void ApplicationInfo::setAppDataDir(const QString &dir) {
        appInfo->appDataDir = dir;
    }

    /*!
        Returns user data directory.

        \li On Mac/Linux, the default path is <tt>\%HOME\%/Documents/\%ORG\%/\%AppName\%</tt>
        \li On Windows, the default path is <tt>\%UserProfile\%/Documents/\%ORG\%/\%AppName\%</tt>
     */
    QString ApplicationInfo::userDataDir() {
        return appInfo->userDataDir;
    }

    /*!
        Sets the user data directory.
    */
    void ApplicationInfo::setUserDataDir(const QString &dir) {
        appInfo->userDataDir = dir;
    }

    /*!
        Returns the application temporary directory.

        \li On Mac/Linux, the default path is <tt>\%TMPDIR\%</tt>
        \li On Windows, the default path is <tt>\%TEMP\%</tt>
    */
    QString ApplicationInfo::tempDir() {
        return appInfo->tempDir;
    }

    /*!
        Sets the application temporary directory.
    */
    void ApplicationInfo::setTempDir(const QString &dir) {
        appInfo->tempDir = dir;
    }

    /*!
        Returns the library directory.

        \li On Mac/Linux, the default path is <tt>\%AppPath\%/../Frameworks</tt>
        \li On Windows, the default path is <tt>\%AppPath\%/../lib</tt>
    */
    QString ApplicationInfo::libDir() {
        return appInfo->libDir;
    }

    /*!
        Sets the library directory.
    */
    void ApplicationInfo::setLibDir(const QString &dir) {
        appInfo->libDir = dir;
    }

    /*!
        Returns the share directory.

        \li On Mac, the default path is <tt>\%AppPath\%/../Resources</tt>
        \li On Windows/Linux, the default path is <tt>\%AppPath\%/../share</tt>
    */
    QString ApplicationInfo::shareDir() {
        return appInfo->shareDir;
    }

    /*!
        Sets the share directory.
    */
    void ApplicationInfo::setShareDir(const QString &dir) {
        appInfo->shareDir = dir;
    }

    /*!
        Returns the application's share directory.

        \li On Mac, the default path is <tt>\%ShareDir\%</tt>
        \li On Windows/Linux, the default path is <tt>\%ShareDir\%/\%AppName\%</tt>
    */
    QString ApplicationInfo::appShareDir() {
        return appInfo->appShareDir;
    }

    /*!
        Sets the application's share directory.
    */
    void ApplicationInfo::setAppShareDir(const QString &dir) {
        appInfo->appShareDir = dir;
    }

    /*!
        Returns the application's plugins directory.

        \li On Mac, the default path is <tt>\%AppPath\%/../Plugins</tt>
        \li On Windows/Linux, the default path is <tt>\%LibDir\%/\%AppName\%/plugins</tt>
    */
    QString ApplicationInfo::appPluginsDir() {
        return appInfo->appPluginsDir;
    }

    /*!
        Sets the application's plugins directory.
    */
    void ApplicationInfo::setAppPluginsDir(const QString &dir) {
        appInfo->appPluginsDir = dir;
    }

}