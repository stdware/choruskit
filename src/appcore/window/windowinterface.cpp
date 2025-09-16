#include "windowinterface.h"
#include "windowinterface_p.h"

#include "coreinterfacebase.h"
#include "windowsystem_p.h"

#include <QDebug>
#include <QFileInfo>
#include <QMimeData>
#include <QActionEvent>
#include <QWindow>
#include <QApplication>

static const int DELAYED_INITIALIZE_INTERVAL = 5; // ms

QT_SPECIALIZE_STD_HASH_TO_CALL_QHASH_BY_CREF(QKeySequence)

namespace Core {

#define myWarning (qWarning().nospace() << "Core::WindowInterface::" << __func__ << "():").space()

    class WindowEventFilter : public QObject {
    public:
        explicit WindowEventFilter(WindowInterfacePrivate *d, QWindow *w);
        ~WindowEventFilter();

        WindowInterfacePrivate *d;
        QWindow *w;

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;
    };

    WindowEventFilter::WindowEventFilter(WindowInterfacePrivate *d, QWindow *w) : QObject(d), d(d), w(w) {
        w->installEventFilter(this);
    }

    WindowEventFilter::~WindowEventFilter() {
    }

    bool WindowEventFilter::eventFilter(QObject *obj, QEvent *event) {
        if (obj == w) {
            switch (event->type()) {
                case QEvent::Close:
                    // Auto-destroy window behavior like WA_DeleteOnClose
                    if (event->isAccepted()) {
                        w->deleteLater();
                    }
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

    WindowInterfaceAddOnPrivate::WindowInterfaceAddOnPrivate() {
    }

    WindowInterfaceAddOnPrivate::~WindowInterfaceAddOnPrivate() = default;

    void WindowInterfaceAddOnPrivate::init() {
    }

    WindowInterfaceAddOn::WindowInterfaceAddOn(QObject *parent) : WindowInterfaceAddOn(*new WindowInterfaceAddOnPrivate(), parent) {
    }

    WindowInterfaceAddOn::~WindowInterfaceAddOn() {
    }

    WindowInterface *WindowInterfaceAddOn::windowHandle() const {
        Q_D(const WindowInterfaceAddOn);
        return static_cast<WindowInterface *>(d->host);
    }

    WindowInterfaceAddOn::WindowInterfaceAddOn(WindowInterfaceAddOnPrivate &d, QObject *parent)
        : ExecutiveInterfaceAddOn(d, parent) {
        d.init();
    }

    WindowInterfacePrivate::WindowInterfacePrivate() {
        closeAsExit = true;
        window = nullptr;
    }

    WindowInterfacePrivate::~WindowInterfacePrivate() {
    }

    void WindowInterfacePrivate::init() {
    }

    void WindowInterfacePrivate::load(bool enableDelayed) {
        Q_Q(WindowInterface);

        // Create window
        auto win = q->createWindow(nullptr);

        // Ensure closing window when quit
        connect(qApp, &QApplication::aboutToQuit, win, &QWindow::close);

        q->setWindow(win);
        winFilter = new WindowEventFilter(this, win);

        ExecutiveInterfacePrivate::load(enableDelayed);

        CoreInterfaceBase::instance()->windowSystem()->d_func()->windowCreated(q);

        win->show();
    }

    void WindowInterfacePrivate::quit() {
        Q_Q(WindowInterface);

        auto w = q->window();
        // FIXME: QGuiApplication::quitOnLastWindowClosed seems not to work if the last window is closed when it is invisible
        // if (w && w->isVisible())
        //     w->hide();

        if (w) {
            w->deleteLater();
        }

        if (CoreInterfaceBase::instance()) { // CoreInterfaceBase might have already been destroyed at this point of time
            CoreInterfaceBase::windowSystem()->d_func()->windowAboutToDestroy(q);
        }

        ExecutiveInterfacePrivate::quit();

        q->deleteLater();
    }

    WindowInterface::~WindowInterface() {
    }

    bool WindowInterface::closeAsExit() const {
        Q_D(const WindowInterface);
        return d->closeAsExit;
    }

    void WindowInterface::setCloseAsExit(bool on) {
        Q_D(WindowInterface);
        d->closeAsExit = on;
    }

    QWindow *WindowInterface::window() const {
        Q_D(const WindowInterface);
        return d->window;
    }

    void WindowInterface::setWindow(QWindow *w) {
        Q_D(WindowInterface);
        if (d->window != w) {
            d->window = w;
            emit windowChanged(w);
        }
    }

    WindowInterface::WindowInterface(QObject *parent) : WindowInterface(*new WindowInterfacePrivate(), parent) {
    }

    WindowInterface::WindowInterface(WindowInterfacePrivate &d, QObject *parent) : ExecutiveInterface(d, parent) {
        d.init();
    }

}
