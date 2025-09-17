#ifndef TRANSLATIONMANAGER_P_H
#define TRANSLATIONMANAGER_P_H

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the ChorusKit API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

#include <QMap>
#include <QSet>
#include <QTranslator>

#include <CoreApi/translationmanager.h>

namespace Core {

    class CKAPPCORE_EXPORT TranslationManagerPrivate {
        Q_DECLARE_PUBLIC(TranslationManager)
    public:
        TranslationManagerPrivate();
        virtual ~TranslationManagerPrivate();

        void init();

        void scanTranslations() const;

        void insertTranslationFiles_helper(const QMap<QString, QStringList> &map) const;

        TranslationManager *q_ptr;

        QSet<QString> translationPaths;
        QList<QTranslator *> translators;

        mutable bool qmFilesDirty;
        mutable QMap<QString, QStringList> qmFiles;
    };

}

#endif // TRANSLATIONMANAGER_P_H