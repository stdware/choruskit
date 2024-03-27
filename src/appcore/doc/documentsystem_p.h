#ifndef DOCUMENTSYSTEM_P_H
#define DOCUMENTSYSTEM_P_H

#include <QMCore/qmchronomap.h>
#include <QMCore/qmchronoset.h>

#include <CoreApi/private/documentwatcher_p.h>
#include <CoreApi/documentsystem.h>

namespace Core {

    class DocumentSystemPrivate : public DocumentWatcherPrivate {
        Q_OBJECT
        Q_DECLARE_PUBLIC(DocumentSystem)
    public:
        DocumentSystemPrivate();
        ~DocumentSystemPrivate();

        void init();

        void readSettings();
        void saveSettings() const;

        void saveOpenFileSettings() const;

        QMChronoMap<QString, DocumentSpec *> docSpecs;
        QHash<QString, QMChronoSet<DocumentSpec *>> extensionsMap;
        QHash<QString, QString> preferredExtensionIdMap;

        QStringList m_recentFiles;
        QStringList m_recentDirs;

        QMChronoMap<IDocument *, QJsonObject> docInfos;

        mutable QString openFileLastVisitDir;
        mutable QString openDirLastVisitDir;
        mutable QString saveFileLastVisitDir;
        mutable bool selectAllWhenRecover;

        mutable bool openFilesSaved;
        void postSaveOpenFilesTask();

    private:
        void _q_docInfoChanged();
        void _q_documentDestroyed();
    };

}

#endif // DOCUMENTSYSTEM_P_H
