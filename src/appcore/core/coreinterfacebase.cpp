#include "coreinterfacebase.h"
#include "coreinterfacebase_p.h"

#include <QTimer>
#include <QtCore/QFileInfo>
#include <QtCore/QLoggingCategory>
#include <QtGui/QGuiApplication>

#include <CoreApi/windowsystem.h>
#include <CoreApi/settingcatalog.h>
#include <CoreApi/recentfilecollection.h>

namespace Core {

    Q_STATIC_LOGGING_CATEGORY(lcCoreInterfaceBase, "ck.coreinterfacebase")

    CoreInterfaceBasePrivate::CoreInterfaceBasePrivate() {
    }

    CoreInterfaceBasePrivate::~CoreInterfaceBasePrivate() {
    }

    void CoreInterfaceBasePrivate::init() {
        Q_Q(CoreInterfaceBase);

        windowSystem = new WindowSystem(q);
        recentFileCollection = new RecentFileCollection(q);
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
        qCInfo(lcCoreInterfaceBase) << "Manually exit" << exitCode;
        QTimer::singleShot(0, [exitCode] {
            qCDebug(lcCoreInterfaceBase) << "Calling QCoreApplication::exit" << exitCode;
            QCoreApplication::exit(exitCode);
        });
    }
    void CoreInterfaceBase::restartApplication(int exitCode) {
        qCInfo(lcCoreInterfaceBase) << "Restarting application" << exitCode;
        qApp->setProperty("restart", true);
        exitApplicationGracefully(exitCode);
    }

    WindowSystem *CoreInterfaceBase::windowSystem() {
        Q_ASSERT(m_instance);
        return m_instance->d_func()->windowSystem;
    }

    RecentFileCollection *CoreInterfaceBase::recentFileCollection() {
        Q_ASSERT(m_instance);
        return m_instance->d_func()->recentFileCollection;
    }

    SettingCatalog *CoreInterfaceBase::settingCatalog() {
        Q_ASSERT(m_instance);
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
