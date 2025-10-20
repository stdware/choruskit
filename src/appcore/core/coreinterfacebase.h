#ifndef COREINTERFACEBASE_H
#define COREINTERFACEBASE_H

#include <QObject>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class CoreInterfaceBasePrivate;

    class WindowSystem;
    class RecentFileCollection;
    class SettingCatalog;

    class CKAPPCORE_EXPORT CoreInterfaceBase : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(CoreInterfaceBase)
        Q_PROPERTY(Core::WindowSystem *windowSystem READ windowSystem CONSTANT)
        Q_PROPERTY(Core::RecentFileCollection *recentFileCollection READ recentFileCollection CONSTANT)
        Q_PROPERTY(Core::SettingCatalog *settingCatalog READ settingCatalog CONSTANT)
    public:
        explicit CoreInterfaceBase(QObject *parent = nullptr);
        ~CoreInterfaceBase() override;

        static CoreInterfaceBase *instance();

        Q_INVOKABLE static void exitApplicationGracefully(int exitCode = 0);
        Q_INVOKABLE static void restartApplication(int exitCode = 0);

    public:
        static WindowSystem *windowSystem();
        static RecentFileCollection *recentFileCollection();
        static SettingCatalog *settingCatalog();

    protected:
        CoreInterfaceBase(CoreInterfaceBasePrivate &d, QObject *parent = nullptr);

        QScopedPointer<CoreInterfaceBasePrivate> d_ptr;
    };

}

#endif // COREINTERFACEBASE_H
