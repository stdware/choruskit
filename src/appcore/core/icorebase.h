#ifndef ICOREBASE_H
#define ICOREBASE_H

#include <QObject>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class ICoreBasePrivate;

    class WindowSystem;
    class DocumentSystem;
    class SettingCatalog;

    class CKAPPCORE_EXPORT ICoreBase : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ICoreBase)
        Q_PROPERTY(Core::WindowSystem *windowSystem READ windowSystem CONSTANT)
        Q_PROPERTY(Core::DocumentSystem *documentSystem READ documentSystem CONSTANT)
        Q_PROPERTY(Core::SettingCatalog *settingCatalog READ settingCatalog CONSTANT)
    public:
        explicit ICoreBase(QObject *parent = nullptr);
        ~ICoreBase() override;

        static ICoreBase *instance();

        Q_INVOKABLE static void exitApplicationGracefully(int exitCode = 0);
        Q_INVOKABLE static void restartApplication(int exitCode = 0);

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