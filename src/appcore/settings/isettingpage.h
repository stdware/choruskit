#ifndef ISETTINGPAGE_H
#define ISETTINGPAGE_H

#include <QObject>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class ISettingPagePrivate;

    class CKAPPCORE_EXPORT ISettingPage : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ISettingPage)
        Q_PROPERTY(QString id READ id CONSTANT)
        Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
        Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
        Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)
        Q_PROPERTY(QString sortKeyword READ sortKeyword CONSTANT)
        Q_PROPERTY(QList<ISettingPage *> pages READ pages)
        Q_PROPERTY(ISettingPage *parentPage READ parentPage)
        Q_PROPERTY(QObject *widget READ widget)
    public:
        ISettingPage(const QString &id, QObject *parent = nullptr);
        ~ISettingPage();

    public:
        QString id() const;

        QString title() const;
        void setTitle(const QString &title);

        QString description() const;
        void setDescription(const QString &description);

        bool dirty() const;
        Q_INVOKABLE void markDirty();

        bool addPage(ISettingPage *page);
        bool removePage(ISettingPage *page);
        bool removePage(const QString &id);

        ISettingPage *page(const QString &id) const;
        QList<ISettingPage *> pages() const;
        QList<ISettingPage *> allPages() const;

        inline ISettingPage *parentPage() const;

    public:
        virtual QString sortKeyword() const;

        Q_INVOKABLE virtual bool matches(const QString &word) const;

        // Abstraction for QtWidgets and QtQuick
        virtual QObject *widget() = 0;

        virtual bool accept() = 0;
        virtual void finish();

    Q_SIGNALS:
        void titleChanged(const QString &title);
        void descriptionChanged(const QString &description);
        void dirtyChanged(bool dirty);

        void pageAdded(ISettingPage *page);
        void pageRemoved(ISettingPage *page);

    protected:
        ISettingPage(ISettingPagePrivate &d, const QString &id, QObject *parent = nullptr);

        QScopedPointer<ISettingPagePrivate> d_ptr;
    };

    ISettingPage *ISettingPage::parentPage() const {
        return qobject_cast<ISettingPage *>(parent());
    }

}

#endif // ISETTINGPAGE_H