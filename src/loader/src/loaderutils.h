#ifndef CHORUSKIT_LOADERUTILS_H
#define CHORUSKIT_LOADERUTILS_H

#include <QtCore/QObject>
#include <QtGui/QScreen>

namespace Loader {

    QFont systemDefaultFont();

    QFont systemDefaultFontWithDpi(double dpi);

    QColor parseColor(const QString &s);

}

#endif // CHORUSKIT_LOADERUTILS_H