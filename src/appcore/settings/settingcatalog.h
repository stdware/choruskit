#ifndef SETTINGCATALOG_H
#define SETTINGCATALOG_H

#include <CoreApi/isettingpage.h>

namespace Core {

    class SettingCatalogPrivate;

    class CKAPPCORE_EXPORT SettingCatalog : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(SettingCatalog)
        Q_PROPERTY(QList<ISettingPage *> pages READ pages)
        Q_PROPERTY(QList<ISettingPage *> allPages READ allPages)
    public:
        explicit SettingCatalog(QObject *parent = nullptr);
        ~SettingCatalog();

    public:
        bool addPage(ISettingPage *page);
        bool removePage(ISettingPage *page);
        bool removePage(const QString &id);

        ISettingPage *page(const QString &id) const;
        QList<ISettingPage *> pages() const;
        QList<ISettingPage *> pages(const QString &id) const;
        QList<ISettingPage *> allPages() const;

    Q_SIGNALS:
        void titleChanged(ISettingPage *page, const QString &title);
        void descriptionChanged(ISettingPage *page, const QString &description);

        void pageAdded(ISettingPage *page);
        void pageRemoved(ISettingPage *page);

    protected:
        SettingCatalog(SettingCatalogPrivate &d, QObject *parent = nullptr);

        QScopedPointer<SettingCatalogPrivate> d_ptr;
    };

}

#endif // SETTINGCATALOG_H
