#ifndef ISETTINGPAGEPRIVATE_H
#define ISETTINGPAGEPRIVATE_H

#include <stdcorelib/linked_map.h>

#include <CoreApi/isettingpage.h>

namespace Core {

    class ISettingPagePrivate {
        Q_DECLARE_PUBLIC(ISettingPage)
    public:
        ISettingPagePrivate();
        virtual ~ISettingPagePrivate();

        void init();

        ISettingPage *q_ptr;

        stdc::linked_map<QString, ISettingPage *> pages;

        QString id;
        QString title;
        QString description;
    };

}

#endif // ISETTINGPAGEPRIVATE_H