#ifndef CHORUSKIT_LOADERUTILS_H
#define CHORUSKIT_LOADERUTILS_H

#include <QtCore/QObject>
#include <QtGui/QScreen>

namespace Loader {

    Q_DECL_CONSTEXPR inline int unitDpi() {
#ifdef Q_OS_MACOS
        return 72;
#else
        return 96;
#endif
    }

    inline double screenDpi(const QScreen *screen) {
#if QT_VERSION_MAJOR < 6
        return screen->logicalDotsPerInch() / unitDpi();
#else
        return screen->devicePixelRatio();
#endif
    }

    bool isUserRoot();

    enum MessageBoxFlag {
        NoIcon,
        Information,
        Question,
        Warning,
        Critical,
    };

    void systemMessageBox(QObject *parent, MessageBoxFlag flag, const QString &title,
                          const QString &text);

    QFont systemDefaultFont();

    QFont systemDefaultFontWithDpi(double dpi);

    QString systemAppDataPath();

    QColor parseColor(const QString &s);

}

#endif // CHORUSKIT_LOADERUTILS_H