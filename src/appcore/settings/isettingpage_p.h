#ifndef ISETTINGPAGEPRIVATE_H
#define ISETTINGPAGEPRIVATE_H

#include <QMCore/qmchronomap.h>

#include <CoreApi/isettingpage.h>

namespace Core {

    class ISettingPagePrivate {
        Q_DECLARE_PUBLIC(ISettingPage)
    public:
        ISettingPagePrivate();
        virtual ~ISettingPagePrivate();

        void init();

        ISettingPage *q_ptr;

        QMChronoMap<QString, ISettingPage *> pages;

        QString id;
        QMDisplayString title;
        QMDisplayString description;
    };

}

#endif // ISETTINGPAGEPRIVATE_H