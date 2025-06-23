#include "iwindow.h"
#include "iwindow_p.h"

#include "icorebase.h"
#include "windowsystem_p.h"

#include <QDebug>
#include <QFileInfo>
#include <QMimeData>
#include <QActionEvent>

#include <private/qwidget_p.h>

static const int DELAYED_INITIALIZE_INTERVAL = 5; // ms

QT_SPECIALIZE_STD_HASH_TO_CALL_QHASH_BY_CREF(QKeySequence)

namespace Core {

#define myWarning(func) (qWarning().nospace() << "Core::IWindow::" << (func) << "():").space()

    class WindowEventFilter : public QObject {
    public:
        explicit WindowEventFilter(IWindowPrivate *d, QWidget *w);
        ~WindowEventFilter();

        IWindowPrivate *d;
        QWidget *w;

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;
    };

    WindowEventFilter::WindowEventFilter(IWindowPrivate *d, QWidget *w) : QObject(d), d(d), w(w) {
        w->installEventFilter(this);
    }

    WindowEventFilter::~WindowEventFilter() {
    }

    bool WindowEventFilter::eventFilter(QObject *obj, QEvent *event) {
        if (obj == w) {
            switch (event->type()) {
                case QEvent::Close:
                    if (d->closeAsExit && event->isAccepted()) {
                        d->quit();
                    }
                    break;
                default:
                    break;
            }
        }
        return QObject::eventFilter(obj, event);
    }

    IWindowAddOnPrivate::IWindowAddOnPrivate() {
    }

    IWindowAddOnPrivate::~IWindowAddOnPrivate() = default;

    void IWindowAddOnPrivate::init() {
    }

    IWindowAddOn::IWindowAddOn(QObject *parent) : IWindowAddOn(*new IWindowAddOnPrivate(), parent) {
    }

    IWindowAddOn::~IWindowAddOn() {
    }

    IWindow *IWindowAddOn::windowHandle() const {
        Q_D(const IWindowAddOn);
        return static_cast<IWindow *>(d->host);
    }

    IWindowAddOn::IWindowAddOn(IWindowAddOnPrivate &d, QObject *parent)
        : IExecutiveAddOn(*new IWindowAddOnPrivate(), parent) {
        d.init();
    }

    IWindowPrivate::IWindowPrivate() {
        closeAsExit = true;
    }

    IWindowPrivate::~IWindowPrivate() {
    }

    void IWindowPrivate::init() {
    }

    void IWindowPrivate::load(bool enableDelayed) {
        Q_Q(IWindow);

        // Create window
        auto win = q->createWindow(nullptr);
        win->setAttribute(Qt::WA_DeleteOnClose);

        // Ensure closing window when quit
        connect(qApp, &QApplication::aboutToQuit, win, &QWidget::close);

        q->setWindow(win);
        winFilter = new WindowEventFilter(this, win);

        IExecutivePrivate::load(enableDelayed);

        ICoreBase::instance()->windowSystem()->d_func()->windowCreated(q);

        win->show();
    }

    void IWindowPrivate::quit() {
        Q_Q(IWindow);

        auto w = q->window();
        if (!w->isHidden())
            w->hide();

        ICoreBase::instance()->windowSystem()->d_func()->windowAboutToDestroy(q);

        IExecutivePrivate::quit();

        q->setWindow(nullptr);
        q->deleteLater();
    }

    IWindow::~IWindow() {
    }

    bool IWindow::closeAsExit() const {
        Q_D(const IWindow);
        return d->closeAsExit;
    }

    void IWindow::setCloseAsExit(bool on) {
        Q_D(IWindow);
        d->closeAsExit = on;
    }

    IWindow::IWindow(QObject *parent) : IWindow(*new IWindowPrivate(), parent) {
    }

    IWindow::IWindow(IWindowPrivate &d, QObject *parent) : IExecutive(d, parent) {
        d.init();
    }

}
