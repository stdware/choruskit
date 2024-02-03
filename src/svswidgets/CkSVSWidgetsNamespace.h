#ifndef CKSVSWIDGETSNAMESPACE_H
#define CKSVSWIDGETSNAMESPACE_H

#include <QObject>

#include <SVSWidgets/CkSVSWidgetsGlobal.h>

namespace SVS {

    Q_NAMESPACE_EXPORT(CKSVSWIDGETS_API)

    enum Role {
        DisplayRole = Qt::DisplayRole,
        DecorationRole = Qt::DecorationRole,

        // Customized
        SubtitleRole = Qt::UserRole + 2000,
        DescriptionRole,
        CategoryRole,
        EnumerationRole,
        SeparatorRole,
        IconSizeRole,
        ObjectPointerRole,
        AlignmentRole,
        InternalDataRole,
        InternalTypeRole,
        KeywordRole,

        UserRole = Qt::UserRole + 4000,
    };

    Q_ENUM_NS(Role)

}

#endif // CKSVSWIDGETSNAMESPACE_H
