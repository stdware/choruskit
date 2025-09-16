#include "coreinterfacebase.h"
#include "coreinterfacebase_p.h"

#include <QTimer>
#include <QtCore/QFileInfo>
#include <QtGui/QGuiApplication>

#include <CoreApi/windowsystem.h>
#include <CoreApi/settingcatalog.h>
#include <CoreApi/documentsystem.h>

namespace Core {

#define myWarning (qWarning().nospace() << "Core::CoreInterfaceBase::" << __func__ << "():").space()

#define checkInstance                                                                              \
    if (!m_instance) {                                                                             \
        myWarning << "Please instantiate the CoreInterfaceBase object first";                              \
        return {};                                                                                 \
    }

    CoreInterfaceBasePrivate::CoreInterfaceBasePrivate() {
    }

    CoreInterfaceBasePrivate::~CoreInterfaceBasePrivate() {
    }

    void CoreInterfaceBasePrivate::init() {
        Q_Q(CoreInterfaceBase);

        windowSystem = new WindowSystem(q);
        documentSystem = new DocumentSystem(q);
        settingCatalog = new SettingCatalog(q);
    }

    static CoreInterfaceBase *m_instance = nullptr;

    CoreInterfaceBase::CoreInterfaceBase(QObject *parent) : CoreInterfaceBase(*new CoreInterfaceBasePrivate(), parent) {
    }

    CoreInterfaceBase::~CoreInterfaceBase() {
        m_instance = nullptr;
    }

    CoreInterfaceBase *CoreInterfaceBase::instance() {
        return m_instance;
    }
    void CoreInterfaceBase::exitApplicationGracefully(int exitCode) {
        QTimer::singleShot(0, [exitCode] {
            QCoreApplication::exit(exitCode);
        });
    }
    void CoreInterfaceBase::restartApplication(int exitCode) {
        qApp->setProperty("restart", true);
        exitApplicationGracefully(exitCode);
    }

    WindowSystem *CoreInterfaceBase::windowSystem() {
        checkInstance;
        return m_instance->d_func()->windowSystem;
    }

    DocumentSystem *CoreInterfaceBase::documentSystem() {
        checkInstance;
        return m_instance->d_func()->documentSystem;
    }

    SettingCatalog *CoreInterfaceBase::settingCatalog() {
        checkInstance;
        return m_instance->d_func()->settingCatalog;
    }

    CoreInterfaceBase::CoreInterfaceBase(CoreInterfaceBasePrivate &d, QObject *parent) : QObject(parent), d_ptr(&d) {
        Q_ASSERT(!m_instance);
        m_instance = this;
        d.q_ptr = this;

        d.init();
    }

}

#include "moc_coreinterfacebase.cpp"
