#ifndef CHORUSKIT_DISPLAYSTRING_H
#define CHORUSKIT_DISPLAYSTRING_H

#include <QPair>
#include <QSharedDataPointer>
#include <QString>
#include <QVariant>

#include <CoreApi/ckappcoreglobal.h>

namespace Core {

    class DisplayStringData;

    class CKAPPCORE_EXPORT DisplayString {
    public:
        enum TranslatePolicy {
            TranslateIgnored = 0,
            TranslateAlways = 1,
            TranslateAlwaysEx = 3,
        };

        using GetText = std::function<QString()>;
        using GetTextEx = std::function<QString(const DisplayString &)>;

        DisplayString() : DisplayString(QString()){};
        DisplayString(const QString &s);
        DisplayString(const GetText &func);
        explicit DisplayString(const GetTextEx &func, void *userdata = nullptr);
        ~DisplayString();

        DisplayString(const DisplayString &other);
        DisplayString(DisplayString &&other) noexcept;

        DisplayString &operator=(const QString &s);
        DisplayString &operator=(const DisplayString &other);
        DisplayString &operator=(DisplayString &&other) noexcept;

        QString text() const;
        TranslatePolicy translatePolicy() const;

        void setTranslateCallback(const GetText &func);
        void setTranslateCallback(const GetTextEx &func);
        void setPlainString(const QString &s);

        QVariant property(const QString &key) const;
        void setProperty(const QString &key, const QVariant &value);
        QVariantHash propertyMap() const;

        inline operator QString() const;

        template <class Func>
        inline DisplayString(Func func) : DisplayString(GetText(func)){};

    private:
        DisplayStringData *d;

        friend class DisplayStringData;
    };

    inline DisplayString::operator QString() const {
        return text();
    }

}

Q_DECLARE_METATYPE(Core::DisplayString)

#endif // CHORUSKIT_DISPLAYSTRING_H