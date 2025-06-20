#include "loaderutils.h"

#ifdef Q_OS_WINDOWS
#  include <ShlObj.h>
#  include <Windows.h>
#elif defined(Q_OS_MACOS)
#  include <CoreFoundation/CoreFoundation.h>
#endif

#include <QtCore/QStandardPaths>
#include <QtGui/QFontDatabase>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtWidgets/QWidget>

#include <QtGui/private/qcssparser_p.h>

namespace Loader {

    /*!
    \fn int unitDpi()

    Returns the system unit dpi value.

    \li on Mac, returns 72
    \li on Windows/Linux, returns 96
*/

    /*!
        Returns \c true if running with Administrator/Root privilege.
    */
    bool isUserRoot() {
#ifdef Q_OS_WINDOWS
        return ::IsUserAnAdmin();
#else
        return geteuid() == 0;
#endif
    }

#if defined(Q_OS_WINDOWS) || defined(Q_OS_MAC)

    static void osMessageBox(void *winHandle, MessageBoxFlag flag, const QString &title,
                             const QString &text) {
#  ifdef Q_OS_WINDOWS
        int winFlag;
        switch (flag) {
            case NoIcon:
                winFlag = MB_OK;
                break;
            case Critical:
                winFlag = MB_ICONERROR;
                break;
            case Warning:
                winFlag = MB_ICONWARNING;
                break;
            case Question:
                winFlag = MB_ICONQUESTION;
                break;
            case Information:
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
            case Critical:
                level = 2;
                break;
            case Warning:
                level = 1;
                break;
            case Question:
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
    void systemMessageBox(QObject *parent, MessageBoxFlag flag, const QString &title,
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
        Returns the system default font.
    */
    QFont systemDefaultFont() {
#if defined(Q_OS_WINDOWS) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        NONCLIENTMETRICSW ncm = {0};
        ncm.cbSize = FIELD_OFFSET(NONCLIENTMETRICSW, lfMessageFont) + sizeof(LOGFONTW);
        SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
        return [](const LOGFONTW &logFont) {
            QFont qFont(QString::fromWCharArray(logFont.lfFaceName));
            qFont.setItalic(logFont.lfItalic);
            if (logFont.lfWeight != FW_DONTCARE)
                qFont.setWeight(QPlatformFontDatabase::weightFromInteger(logFont.lfWeight));
            const qreal logFontHeight = qAbs(logFont.lfHeight);
            qFont.setPixelSize(qAbs(logFont.lfHeight));
            qFont.setUnderline(logFont.lfUnderline);
            qFont.setOverline(false);
            qFont.setStrikeOut(logFont.lfStrikeOut);
            return qFont;
        }(ncm.lfMessageFont);
#else
        QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
        font.setPixelSize(font.pointSize() / 72.0 * unitDpi());
        return font;
#endif
    }

    /*!
        Returns the system default font with dpi scaling.
    */
    QFont systemDefaultFontWithDpi(double dpi) {
        QFont font = systemDefaultFont();
        double ratio =
            (dpi > 0 ? dpi : QGuiApplication::primaryScreen()->logicalDotsPerInch()) / unitDpi();
        font.setPixelSize(int(12 * ratio));
        return font;
    }

    /*!
        Returns the standard AppData location.

        \li On Windows, returns <tt>\%UserProfile\%/AppData</tt>
        \li On Mac/Linux, returns <tt>\%HOME\%/.config</tt>
    */
    QString systemAppDataPath() {
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
            slashName = '/' + qApp->applicationName();
            if (path.endsWith(slashName)) {
                path.chop(slashName.size());
            }
            slashName = '/' + qApp->organizationName();
            if (path.endsWith(slashName)) {
                path.chop(slashName.size());
            }
            return path;
        }();
        return path;
    }

    static QStringList extractFunctionToStringList(const QString &str, bool *ok) {
        int leftParen = str.indexOf('(');
        int rightParen = str.lastIndexOf(')');
        QStringList res;
        if (leftParen > 0 && rightParen > leftParen) {
            if (ok) {
                *ok = true;
            }
            res = QStringList(
                {str.mid(0, leftParen), str.mid(leftParen + 1, rightParen - leftParen - 1)});
        } else {
            if (ok) {
                *ok = false;
            }
        }
        return res;
    }

    /*!
        Parses color value from string in CSS format.
    */
    QColor parseColor(const QString &s) {
        if (s.isEmpty()) {
            return Qt::transparent;
        }

        // Use the feature of QCssParser
        QCss::Declaration dec;
        QCss::Value val;

        bool ok;
        QStringList valueList = extractFunctionToStringList(s, &ok);
        if (ok) {
            val.type = QCss::Value::Function;
            val.variant = valueList;
        } else {
            val.type = QCss::Value::String;
            val.variant = s;
        }

        dec.d->values.push_back(val);
        return dec.colorValue();
    }

}