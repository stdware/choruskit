#ifndef CHORUSKIT_LOADERUTILS_H
#define CHORUSKIT_LOADERUTILS_H

#include <QtCore/QObject>
#include <QtGui/QScreen>

namespace Loader {

    inline double screenDpi(const QScreen *screen) {
#if QT_VERSION_MAJOR < 6
        return screen->logicalDotsPerInch() / unitDpi();
#else
        return screen->devicePixelRatio();
#endif
    }

    enum MessageBoxFlag {
        NoIcon,
        Information,
        Question,
        Warning,
        Critical,
    };

    QFont systemDefaultFont();

    QFont systemDefaultFontWithDpi(double dpi);

    QColor parseColor(const QString &s);

}

#endif // CHORUSKIT_LOADERUTILS_H