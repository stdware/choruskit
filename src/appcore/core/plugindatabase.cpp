#include "plugindatabase.h"

#include <tuple>

#include "objectpool_p.h"

namespace Core {

#define myWarning (qWarning().nospace() << "Core::PluginDatabase::" << __func__ << "():").space()

#define checkInstanceV                                                                             \
    if (!m_instance) {                                                                             \
        myWarning << "Please instantiate the PluginDatabase object first";                         \
        return;                                                                                    \
    }

#define checkInstance                                                                              \
    if (!m_instance) {                                                                             \
        myWarning << "Please instantiate the PluginDatabase object first";                         \
        return {};                                                                                 \
    }

    class PluginDatabasePrivate : public ObjectPoolPrivate {
        Q_DECLARE_PUBLIC(PluginDatabase)
    public:
        PluginDatabasePrivate() {
            std::ignore = PluginDatabase::startTime();
        }

        QSettings *settings = nullptr;
        QSettings *globalSettings = nullptr;

        QSplashScreen *splash = nullptr;
    };

    static PluginDatabase *m_instance = nullptr;

    PluginDatabase::PluginDatabase(QObject *parent)
        : ObjectPool(*new PluginDatabasePrivate(), parent) {
        m_instance = this;
    }

    PluginDatabase::~PluginDatabase() = default;

    QDateTime PluginDatabase::startTime() {
        static QDateTime start = QDateTime::currentDateTime();
        return start;
    }

    QSettings *PluginDatabase::settings() {
        checkInstance;
        return m_instance->d_func()->settings;
    }

    void PluginDatabase::setSettings(QSettings *settings) {
        checkInstanceV;

        auto &settingsRef = m_instance->d_func()->settings;
        delete settingsRef;

        settingsRef = settings;
        settings->setParent(m_instance);
    }

    QSettings *PluginDatabase::globalSettings() {
        checkInstance;
        return m_instance->d_func()->globalSettings;
    }

    void PluginDatabase::setGlobalSettings(QSettings *settings) {
        checkInstanceV;

        auto &settingsRef = m_instance->d_func()->globalSettings;
        delete settingsRef;

        settingsRef = settings;
        settings->setParent(m_instance);
    }

    QSplashScreen *PluginDatabase::splash() {
        checkInstance;
        return m_instance->d_func()->splash;
    }

    void PluginDatabase::setSplash(QSplashScreen *splash) {
        checkInstanceV;

        auto &splashRef = m_instance->d_func()->splash;
        if (splashRef) {
            myWarning << "Ignores the new splash screen because there is already one set";
            return;
        }
        splashRef = splash;
    }

    PluginDatabase *PluginDatabase::instance() {
        return m_instance;
    }

}