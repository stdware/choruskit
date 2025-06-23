#include "displaystring.h"

#include <QCoreApplication>
#include <QDebug>

namespace Core {

    // The internal class should be transparent in an anonymous namespace.
    namespace {
        class BaseString;
    }

    class DisplayStringData {
    public:
        DisplayString *q;
        BaseString *str;
        QVariantHash properties;

        explicit DisplayStringData(const QString &s, DisplayString *q);
        explicit DisplayStringData(DisplayString::GetText func, DisplayString *q);
        explicit DisplayStringData(DisplayString::GetTextEx func, void *userdata,
                                     DisplayString *q);
        explicit DisplayStringData(BaseString *str, const QVariantHash &properties,
                                     DisplayString *q);
        ~DisplayStringData();
    };

    namespace {

        class BaseString {
        public:
            explicit BaseString(DisplayString::TranslatePolicy policy, DisplayStringData *q)
                : p(policy), q(q){};
            virtual ~BaseString() = default;

            virtual QString text() const = 0;
            virtual BaseString *clone(DisplayStringData *q) const = 0;

            DisplayString::TranslatePolicy p;
            DisplayStringData *q;
        };

        class PlainString : public BaseString {
        public:
            explicit PlainString(QString s, DisplayStringData *q)
                : BaseString(DisplayString::TranslateIgnored, q), s(std::move(s)){};

            QString text() const override {
                return s;
            }
            BaseString *clone(DisplayStringData *q) const override {
                return new PlainString(s, q);
            }

            QString s;
        };

        class CallbackString : public BaseString {
        public:
            explicit CallbackString(DisplayString::GetText func, DisplayStringData *q)
                : BaseString(DisplayString::TranslateAlways, q), func(std::move(func)){};

            QString text() const override {
                return func();
            }

            BaseString *clone(DisplayStringData *q) const override {
                return new CallbackString(func, q);
            }

            DisplayString::GetText func;
        };

        class CallbackExString : public BaseString {
        public:
            explicit CallbackExString(DisplayString::GetTextEx func, DisplayStringData *q)
                : BaseString(DisplayString::TranslateAlwaysEx, q), func(std::move(func)){};

            QString text() const override {
                return func(*(q->q));
            }
            BaseString *clone(DisplayStringData *q) const override {
                return new CallbackExString(func, q);
            }

            DisplayString::GetTextEx func;
        };

    }

    DisplayStringData::DisplayStringData(const QString &s, DisplayString *q)
        : q(q), str(new PlainString(s, this)) {
    }

    DisplayStringData::DisplayStringData(DisplayString::GetText func, DisplayString *q)
        : q(q), str(func ? decltype(str)(new CallbackString(std::move(func), this))
                         : decltype(str)(new PlainString({}, this))) {
    }

    DisplayStringData::DisplayStringData(DisplayString::GetTextEx func, void *userdata,
                                             DisplayString *q)
        : q(q), str(func ? decltype(str)(new CallbackExString(std::move(func), this))
                         : decltype(str)(new PlainString({}, this))) {
    }

    DisplayStringData::DisplayStringData(BaseString *str, const QVariantHash &properties,
                                             DisplayString *q)
        : q(q), str(str->clone(this)), properties(properties) {
    }

    DisplayStringData::~DisplayStringData() {
        delete str;
    }

    /*!
        \class DisplayString
        \brief A wrapper of QString that always returns a translated string.
    */

    /*!
        \typedef DisplayString::GetText
        \brief Translation callback
    */

    /*!
        \typedef DisplayString::GetTextEx
        \brief Advanced translation callback
    */

    /*!
        Constructs from a plain string.
    */
    DisplayString::DisplayString(const QString &s) : d(new DisplayStringData(s, this)) {
    }

    /*!
        Constructs from a translation callback, you should call QCoreApplication::tr() in this
       callback.
    */
    DisplayString::DisplayString(const DisplayString::GetText &func)
        : d(new DisplayStringData(func, this)) {
    }

    /*!
        Constructs from a translation callback that provides the DisplayString instance, you may
       use the property map in this callback.
    */
    DisplayString::DisplayString(const GetTextEx &func, void *userdata)
        : d(new DisplayStringData(func, userdata, this)) {
    }

    /*!
        Destructor.
    */
    DisplayString::~DisplayString() {
        delete d;
    }

    DisplayString::DisplayString(const DisplayString &other)
        : d(new DisplayStringData(other.d->str, other.d->properties, this)) {
    }

    DisplayString::DisplayString(DisplayString &&other) noexcept : d(other.d) {
        other.d = nullptr;
        d->q = this;
    }

    /*!
        Sets the DisplayString as the given plain string.
    */
    DisplayString &DisplayString::operator=(const QString &s) {
        setPlainString(s);
        return *this;
    }

    DisplayString &DisplayString::operator=(const DisplayString &other) {
        if (&other == this) {
            return *this;
        }

        d = new DisplayStringData(other.d->str, other.d->properties, this);
        return *this;
    }

    DisplayString &DisplayString::operator=(DisplayString &&other) noexcept {
        if (&other == this) {
            return *this;
        }

        d = other.d;
        other.d = nullptr;
        d->q = this;
        return *this;
    }

    /*!
        Returns the plain string or translated string.
    */
    QString DisplayString::text() const {
        return d->str->text();
    }

    /*!
        Returns the translation policy.
    */
    DisplayString::TranslatePolicy DisplayString::translatePolicy() const {
        return d->str->p;
    }

    /*!
        Assigns translation callback, the translation policy may be changed.
    */
    void DisplayString::setTranslateCallback(const DisplayString::GetText &func) {
        if (!func) {
            setPlainString({});
            return;
        }

        if (d->str->p != TranslateAlways) {
            delete d->str;
            d->str = new CallbackString(func, d);
        } else {
            auto str = static_cast<CallbackString *>(d->str);
            str->func = func;
        }
    }

    /*!
        Assigns translation callback, the translation policy may be changed.
    */
    void DisplayString::setTranslateCallback(const DisplayString::GetTextEx &func) {
        if (!func) {
            setPlainString({});
            return;
        }

        if (d->str->p != TranslateAlwaysEx) {
            delete d->str;
            d->str = new CallbackExString(func, d);
        } else {
            auto *str = static_cast<CallbackExString *>(d->str);
            str->func = func;
        }
    }

    /*!
        Assigns plain string, the translation policy may be changed.
    */
    void DisplayString::setPlainString(const QString &s) {
        if (d->str->p != TranslateIgnored) {
            delete d->str;
            d->str = new PlainString(s, d);
        } else {
            auto str = static_cast<PlainString *>(d->str);
            str->s = s;
        }
    }

    /*!
        Gets the property of the given key.
    */
    QVariant DisplayString::property(const QString &key) const {
        return d->properties.value(key);
    }

    /*!
        Sets the property of the given key.
    */
    void DisplayString::setProperty(const QString &key, const QVariant &value) {
        auto &properties = d->properties;
        auto it = properties.find(key);
        if (it == properties.end()) {
            if (!value.isValid())
                return;
            properties.insert(key, value);
        } else {
            if (!value.isValid())
                properties.erase(it);
            else
                it.value() = value;
        }
    }

    /*!
        Returns the property hash map.
    */
    QVariantHash DisplayString::propertyMap() const {
        return d->properties;
    }

    /*!
        \fn DisplayString(Func func)

        Template constructor, \c func must be the type of DisplayString::GetText.
    */

}