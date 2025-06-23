#include "loaderutils.h"

#include <QtGui/QFontDatabase>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtWidgets/QWidget>

#include <QtGui/private/qcssparser_p.h>

#include <CoreApi/applicationinfo.h>

namespace Loader {

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
        font.setPixelSize(font.pointSize() / 72.0 * Core::ApplicationInfo::unitDpi());
        return font;
#endif
    }

    /*!
        Returns the system default font with dpi scaling.
    */
    QFont systemDefaultFontWithDpi(double dpi) {
        QFont font = systemDefaultFont();
        double ratio = (dpi > 0 ? dpi : QGuiApplication::primaryScreen()->logicalDotsPerInch()) /
                       Core::ApplicationInfo::unitDpi();
        font.setPixelSize(int(12 * ratio));
        return font;
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