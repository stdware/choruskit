#ifndef SETTINGCATALOGPRIVATE_H
#define SETTINGCATALOGPRIVATE_H

#include <stdcorelib/linked_map.h>

#include <CoreApi/settingcatalog.h>

namespace Core {

    class SettingCatalogPrivate : public QObject {
        Q_DECLARE_PUBLIC(SettingCatalog)
    public:
        SettingCatalogPrivate();
        ~SettingCatalogPrivate();

        void init();

        SettingCatalog *q_ptr;

        void addPageRecursive(ISettingPage *page);
        void removePageRecursive(ISettingPage *page);

        stdc::linked_map<QString, ISettingPage *> pages;
        QHash<QString, QSet<ISettingPage *>> allPages;

    private:
        void _q_pageTitleChanged(const QString &title);
        void _q_pageDescriptionChanged(const QString &desc);
        void _q_pageAdded(ISettingPage *page);
        void _q_pageRemoved(ISettingPage *page);
    };

}

#endif // SETTINGCATALOGPRIVATE_H