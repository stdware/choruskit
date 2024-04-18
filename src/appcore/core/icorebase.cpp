#include "icorebase.h"
#include "icorebase_p.h"

#include <QApplication>

#include <extensionsystem/pluginmanager.h>

namespace Core {

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

    WindowSystem *ICoreBase::windowSystem() const {
        Q_D(const ICoreBase);
        return d->windowSystem;
    }

    DocumentSystem *ICoreBase::documentSystem() const {
        Q_D(const ICoreBase);
        return d->documentSystem;
    }

    SettingCatalog *ICoreBase::settingCatalog() const {
        Q_D(const ICoreBase);
        return d->settingCatalog;
    }

    ICoreBase::ICoreBase(ICoreBasePrivate &d, QObject *parent) : QObject(parent), d_ptr(&d) {
        m_instance = this;
        d.q_ptr = this;

        d.init();
    }

}