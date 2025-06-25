#include "icorebase.h"
#include "icorebase_p.h"

#include <QtCore/QFileInfo>
#include <QtGui/QGuiApplication>

namespace Core {

#define myWarning (qWarning().nospace() << "Core::ICoreBase::" << __func__ << "():").space()

#define checkInstance                                                                              \
    if (!m_instance) {                                                                             \
        myWarning << "Please instantiate the ICoreBase object first";                              \
        return {};                                                                                 \
    }

    ICoreBasePrivate::ICoreBasePrivate() {
    }

    ICoreBasePrivate::~ICoreBasePrivate() {
    }

    void ICoreBasePrivate::init() {
        Q_Q(ICoreBase);

        windowSystem = new WindowSystem(q);
        documentSystem = new DocumentSystem(q);
        settingCatalog = new SettingCatalog(q);
    }

    static ICoreBase *m_instance = nullptr;

    ICoreBase::ICoreBase(QObject *parent) : ICoreBase(*new ICoreBasePrivate(), parent) {
    }

    ICoreBase::~ICoreBase() {
        m_instance = nullptr;
    }

    ICoreBase *ICoreBase::instance() {
        return m_instance;
    }

    WindowSystem *ICoreBase::windowSystem() {
        checkInstance;
        return m_instance->d_func()->windowSystem;
    }

    DocumentSystem *ICoreBase::documentSystem() {
        checkInstance;
        return m_instance->d_func()->documentSystem;
    }

    SettingCatalog *ICoreBase::settingCatalog() {
        checkInstance;
        return m_instance->d_func()->settingCatalog;
    }

    ICoreBase::ICoreBase(ICoreBasePrivate &d, QObject *parent) : QObject(parent), d_ptr(&d) {
        m_instance = this;
        d.q_ptr = this;

        d.init();
    }

}