#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QObject>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class TranslationManagerPrivate;

    class CKAPPCORE_EXPORT TranslationManager : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(TranslationManager)
    public:
        explicit TranslationManager(QObject *parent = nullptr);
        ~TranslationManager() override;

    public:
        void addTranslationPath(const QString &path);
        void removeTranslationPath(const QString &path);

        QStringList locales() const;
        void setLocale(const QLocale &locale);

    protected:
        TranslationManager(TranslationManagerPrivate &d, QObject *parent = nullptr);

        QScopedPointer<TranslationManagerPrivate> d_ptr;
    };

}

#endif // TRANSLATIONMANAGER_H