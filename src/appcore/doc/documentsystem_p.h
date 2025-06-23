#ifndef DOCUMENTSYSTEM_P_H
#define DOCUMENTSYSTEM_P_H

#include <stdcorelib/linked_map.h>

#include <CoreApi/private/documentwatcher_p.h>
#include <CoreApi/documentsystem.h>

namespace Core {

    class DocumentSystemPrivate : public DocumentWatcherPrivate {
        Q_DECLARE_PUBLIC(DocumentSystem)
    public:
        DocumentSystemPrivate();
        ~DocumentSystemPrivate();

        void init();

        void readSettings();
        void saveSettings() const;

        void saveOpenFileSettings() const;

        stdc::linked_map<QString, DocumentSpec *> docSpecs;
        QHash<QString, stdc::linked_map<DocumentSpec *, int /*NOT USED*/>> extensionsMap;
        QHash<QString, QString> preferredExtensionIdMap;

        QStringList m_recentFiles;
        QStringList m_recentDirs;

        stdc::linked_map<IDocument *, QJsonObject> docInfos;

        mutable QString openFileLastVisitDir;
        mutable QString openDirLastVisitDir;
        mutable QString saveFileLastVisitDir;

        mutable bool openFilesSaved;
        void postSaveOpenFilesTask();

    private:
        void _q_docInfoChanged();
        void _q_documentDestroyed();
    };

}

#endif // DOCUMENTSYSTEM_P_H
