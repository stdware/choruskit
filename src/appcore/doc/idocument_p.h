#ifndef IDOCUMENTPRIVATE_H
#define IDOCUMENTPRIVATE_H

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the ChorusKit API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

#include <QJsonObject>
#include <QSettings>
#include <QSharedPointer>
#include <QTemporaryDir>

#include <CoreApi/documentspec.h>
#include <CoreApi/idocument.h>

namespace Core {

    class CKAPPCORE_EXPORT IDocumentPrivate : public QObject {
        Q_DECLARE_PUBLIC(IDocument)
    public:
        IDocumentPrivate();
        virtual ~IDocumentPrivate();

        void init();

        bool getSpec();

        IDocument *q_ptr;

        QString id;
        DocumentSpec *spec;

        mutable QString errMsg;

        QJsonObject docInfo;
        QString filePath;
        QString preferredDisplayName;
        QString uniqueDisplayName;
        QString autoSaveName;
        bool temporary;
    };

}

#endif // IDOCUMENTPRIVATE_H
