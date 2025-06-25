#ifndef ICOREBASE_H
#define ICOREBASE_H

#include <QObject>
#include <QSettings>

#include <CoreApi/documentsystem.h>
#include <CoreApi/settingcatalog.h>
#include <CoreApi/windowsystem.h>

namespace Core {

    class ICoreBasePrivate;

    class CKAPPCORE_EXPORT ICoreBase : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ICoreBase)
    public:
        explicit ICoreBase(QObject *parent = nullptr);
        ~ICoreBase();

        static ICoreBase *instance();

        // Resource Managers
    public:
        static WindowSystem *windowSystem();
        static DocumentSystem *documentSystem();
        static SettingCatalog *settingCatalog();

    protected:
        ICoreBase(ICoreBasePrivate &d, QObject *parent = nullptr);

        QScopedPointer<ICoreBasePrivate> d_ptr;
    };

}

#endif // ICOREBASE_H