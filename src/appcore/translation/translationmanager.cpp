#include "translationmanager.h"
#include "translationmanager_p.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>

namespace Core {

    TranslationManagerPrivate::TranslationManagerPrivate() {
        qmFilesDirty = false;
    }

    TranslationManagerPrivate::~TranslationManagerPrivate() = default;

    void TranslationManagerPrivate::init() {
    }

    static QMap<QString, QStringList> scanTranslation_helper(const QString &path) {
        QMap<QString, QStringList> res;

        QFileInfoList searchFiles;
        QStringList searchPaths = {path};
        while (!searchPaths.isEmpty()) {
            const QDir dir(searchPaths.takeFirst());
            const QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoSymLinks);
            foreach (const QFileInfo &file, files) {
                if (!file.suffix().compare("qm", Qt::CaseInsensitive)) {
                    searchFiles.append(file);
                }
            }
            const QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
            foreach (const QFileInfo &subdir, dirs)
                searchPaths << subdir.absoluteFilePath();
        }

        QRegularExpression reg(R"((\w+?)_(\w{2})(_\w+|))");
        for (const auto &file : qAsConst(searchFiles)) {
            auto match = reg.match(file.fileName());
            if (!match.hasMatch()) {
                continue;
            }

            QLocale testLocale(match.captured(2) + match.captured(3));
            if (testLocale.language() == QLocale::C)
                continue;
            res[testLocale.name()].append(file.absoluteFilePath());
        }
        return res;
    }

    static QList<QTranslator *> installTranslation_helper(const QStringList &paths) {
        QList<QTranslator *> res;
        for (const auto &file : qAsConst(paths)) {
            auto t = new QTranslator(qApp);
            if (!t->load(file)) {
                delete t;
                continue;
            }
            qApp->installTranslator(t);
            res.append(t);
        }
        return res;
    }

    void TranslationManagerPrivate::scanTranslations() const {
        qmFiles.clear();

        for (const auto &path : qAsConst(translationPaths)) {
            insertTranslationFiles_helper(scanTranslation_helper(path));
        }

        qmFilesDirty = false;
    }

    void TranslationManagerPrivate::insertTranslationFiles_helper(
        const QMap<QString, QStringList> &map) const {
        for (auto it = map.begin(); it != map.end(); ++it) {
            if (it->isEmpty()) {
                continue;
            }
            qmFiles[it.key()].append(it.value());
        }
    }

    /*!
        \class TranslationManager

        The TranslationManager class provides translation path management and locale switching functionality.
        This is a simplified version that focuses only on locale management without update notifications.
    */

    /*!
        Constructor.
    */
    TranslationManager::TranslationManager(QObject *parent)
        : TranslationManager(*new TranslationManagerPrivate(), parent) {
    }

    /*!
        Destructor.
    */
    TranslationManager::~TranslationManager() = default;

    /*!
        Add a directory to the searching paths. Translation files will be scanned from this path.
    */
    void TranslationManager::addTranslationPath(const QString &path) {
        Q_D(TranslationManager);

        if (path.isEmpty())
            return;

        QString canonicalPath = QDir(path).canonicalPath();
        if (canonicalPath.isEmpty())
            return;

        if (d->translationPaths.contains(path))
            return;

        d->translationPaths.insert(path);

        auto map = scanTranslation_helper(path);
        d->insertTranslationFiles_helper(map);
        auto it = map.find(QLocale().name());
        if (it != map.end()) {
            auto translators = installTranslation_helper(it.value());
            d->translators.append(translators);
        }
    }

    /*!
        Remove a directory from the searching paths. This will mark the translation cache as dirty.
    */
    void TranslationManager::removeTranslationPath(const QString &path) {
        Q_D(TranslationManager);

        if (path.isEmpty())
            return;

        QString canonicalPath = QDir(path).canonicalPath();
        if (canonicalPath.isEmpty())
            return;

        auto it = d->translationPaths.find(path);
        if (it == d->translationPaths.end())
            return;

        auto map = scanTranslation_helper(path);
        auto localeFiles = map.value(QLocale().name());
        for (auto &file : localeFiles) {
            for (int i = d->translators.size() - 1; i >= 0; --i) {
                QTranslator *t = d->translators[i];
                if (t && t->isEmpty() == false && t->filePath() == file) {
                    QCoreApplication::removeTranslator(t);
                    delete t;
                    d->translators.removeAt(i);
                }
            }
        }

        d->translationPaths.erase(it);

        d->qmFilesDirty = true;
    }

    /*!
        Returns a list of available locale names.
    */
    QStringList TranslationManager::locales() const {
        Q_D(const TranslationManager);
        if (d->qmFilesDirty) {
            d->scanTranslations();
        }
        return d->qmFiles.keys();
    }

    /*!
        Sets the current locale and installs the corresponding translators.
    */
    void TranslationManager::setLocale(const QLocale &locale) {
        Q_D(TranslationManager);

        if (d->qmFilesDirty) {
            d->scanTranslations();
        }

        // Remove original translators
        qDeleteAll(d->translators);
        d->translators.clear();

        // Install new translators
        auto it = d->qmFiles.find(locale.name());
        if (it != d->qmFiles.end()) {
            auto translators = installTranslation_helper(it.value());
            d->translators.append(translators);
        }
        QLocale::setDefault(locale);
    }

    /*!
        \internal
    */
    TranslationManager::TranslationManager(TranslationManagerPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.init();
    }

}