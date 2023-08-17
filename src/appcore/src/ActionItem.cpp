#include "ActionItem.h"
#include "ActionItem_p.h"

#include "ActionSystem.h"
#include "ICoreBase.h"

#include <QDebug>

namespace Core {

#define myWarning(func) qWarning() << "Core::ActionItem(): "

    static bool m_autoDelete = false;

    ActionItemPrivate::ActionItemPrivate() : q_ptr(nullptr) {
        type = ActionItem::Invalid;
        spec = nullptr;
        autoDelete = m_autoDelete;
    }

    ActionItemPrivate::~ActionItemPrivate() {
    }

    void ActionItemPrivate::init() {
    }

    bool ActionItemPrivate::getSpec() {
        spec = ICoreBase::instance()->actionSystem()->action(id);
        if (!spec) {
            myWarning(__func__) << "action is not registered to ActionSystem:" << id;
            return false;
        }

        connect(spec, &ActionSpec::shortcutsChanged, this, &ActionItemPrivate::_q_actionShortcutsChanged);
        connect(spec, &ActionSpec::iconChanged, this, &ActionItemPrivate::_q_actionIconChanged);

        _q_actionShortcutsChanged();
        _q_actionIconChanged();

        return true;
    }

    void ActionItemPrivate::_q_actionShortcutsChanged() {
        if (type != ActionItem::Action) {
            return;
        }
        auto shortcuts = spec->cachedShortcuts();
        if (shortcuts.isEmpty())
            shortcuts = spec->shortcuts();
        action->setShortcuts(shortcuts);
    }

    void ActionItemPrivate::_q_actionIconChanged() {
        Q_Q(ActionItem);
        auto icon = spec->cachedIcon().icon();
        if (icon.isNull())
            icon = spec->icon();
        q->setIcon(icon);
    }

    ActionItem::ActionItem(const QString &id, QAction *action, QObject *parent)
        : ActionItem(*new ActionItemPrivate(), id, parent) {
        Q_D(ActionItem);

        if (!action) {
            myWarning(__func__) << "trying to wrap null action";
            return;
        }
        if (action->inherits("QWidgetAction")) {
            myWarning(__func__) << "trying to add widget action which is not supported";
            return;
        }
        d->type = Action;
        d->action = action;

        if (!d->getSpec()) {
            return;
        }

        action->setProperty("action-id", id);
    }

    class WidgetAction : public QWidgetAction {
    public:
        explicit WidgetAction(const std::function<QWidget *(QWidget *)> &factory, const QString &id,
                              QObject *parent = nullptr)
            : QWidgetAction(parent), fac(factory) {
        }
        ~WidgetAction() {
        }

    protected:
        QWidget *createWidget(QWidget *parent) override {
            auto w = fac(parent);
            w->setProperty("action-id", id);
            return w;
        }

        std::function<QWidget *(QWidget *)> fac;
        QString id;

        friend class ActionItem;
    };

    ActionItem::ActionItem(const QString &id, const std::function<QWidget *(QWidget *)> &factory, QObject *parent) {
        Q_D(ActionItem);

        if (!factory) {
            myWarning(__func__) << "trying to wrap null widget factory";
            return;
        }

        d->type = Widget;
        d->widgetAction = new WidgetAction(factory, id, this);

        if (!d->getSpec()) {
            return;
        }
    }

    ActionItem::ActionItem(const QString &id, QMenu *menu, QObject *parent)
        : ActionItem(*new ActionItemPrivate(), id, parent) {
        Q_D(ActionItem);

        if (!menu) {
            myWarning(__func__) << "trying to wrap null menu";
            return;
        }
        d->type = Menu;
        d->menu = menu;

        if (!d->getSpec()) {
            return;
        }

        menu->setProperty("action-id", id);
        menu->menuAction()->setProperty("action-id", id);
    }

    ActionItem::~ActionItem() {
        Q_D(ActionItem);

        if (d->autoDelete) {
            QObject *obj = nullptr;
            switch (d->type) {
                case Action:
                    obj = d->action;
                    break;
                case Menu:
                    obj = d->menu;
                    break;
                case Widget:
                    obj = d->widgetAction;
                    break;
                default:
                    break;
            }
            if (obj && !obj->parent()) {
                obj->deleteLater();
            }
        }
    }

    QString ActionItem::id() const {
        Q_D(const ActionItem);
        return d->id;
    }

    ActionItem::Type ActionItem::type() const {
        Q_D(const ActionItem);
        return d->type;
    }

    ActionSpec *ActionItem::spec() const {
        Q_D(const ActionItem);
        return d->spec;
    }

    QAction *ActionItem::action() const {
        Q_D(const ActionItem);
        return d->action;
    }

    QMenu *ActionItem::menu() const {
        Q_D(const ActionItem);
        return d->menu;
    }

    QList<QWidget *> ActionItem::widgets() const {
        Q_D(const ActionItem);
        if (!d->widgetAction)
            return {};
        return static_cast<WidgetAction *>(d->widgetAction.data())->createdWidgets();
    }

    QIcon ActionItem::icon() const {
        Q_D(const ActionItem);
        QIcon res;
        switch (d->type) {
            case Action:
                res = d->action->icon();
                break;
            case Menu:
                res = d->menu->icon();
                break;
            case Widget:
                res = d->widgetAction->icon();
                break;
            default:
                break;
        }
        return res;
    }
    void ActionItem::setIcon(const QIcon &icon) {
        Q_D(ActionItem);

        switch (d->type) {
            case Action:
                d->action->setIcon(icon);
                break;
            case Menu:
                d->menu->setIcon(icon);
                break;
            case Widget:
                d->widgetAction->setIcon(icon);
                break;
            default:
                break;
        }
    }

    QString ActionItem::text() const {
        Q_D(const ActionItem);

        QString res;
        switch (d->type) {
            case Action:
                res = d->action->text();
                break;
            case Menu:
                res = d->menu->title();
                break;
            case Widget:
                res = d->widgetAction->text();
                break;
            default:
                break;
        }
        return res;
    }

    void ActionItem::setText(const QString &text) {
        Q_D(ActionItem);
        switch (d->type) {
            case Action:
                d->action->setText(text);
                break;
            case Menu:
                d->menu->setTitle(text);
                break;
            case Widget:
                d->widgetAction->setText(text);
                break;
            default:
                break;
        }
    }

    bool ActionItem::enabled() const {
        Q_D(const ActionItem);

        bool res = false;
        switch (d->type) {
            case Action:
                res = d->action->isEnabled();
                break;
            case Menu:
                res = d->menu->isEnabled();
                break;
            case Widget:
                res = d->widgetAction->isEnabled();
                break;
            default:
                break;
        }
        return res;
    }

    void ActionItem::setEnabled(bool enabled) {
        Q_D(ActionItem);

        switch (d->type) {
            case Action:
                d->action->setEnabled(enabled);
                break;
            case Menu:
                d->menu->setEnabled(enabled);
                break;
            case Widget:
                d->widgetAction->setEnabled(enabled);
                break;
            default:
                break;
        }
    }

    bool ActionItem::autoDelete() const {
        Q_D(const ActionItem);
        return d->autoDelete;
    }

    void ActionItem::setAutoDelete(bool autoDelete) {
        Q_D(ActionItem);
        d->autoDelete = autoDelete;
    }

    QString ActionItem::commandName() const {
        Q_D(const ActionItem);
        return d->spec ? d->spec->commandName() : QString();
    }

    QString ActionItem::commandDisplayName() const {
        Q_D(const ActionItem);
        if (!d->specificName.isEmpty())
            return d->specificName;

        return d->spec ? d->spec->commandDisplayName() : QString();
    }

    QString ActionItem::commandSpecificName() const {
        Q_D(const ActionItem);
        return d->specificName;
    }

    void ActionItem::setCommandSpecificName(const QString &commandSpecificName) {
        Q_D(ActionItem);
        d->specificName = commandSpecificName;
    }

    QPair<QString, QString> ActionItem::commandCheckedDescription() const {
        Q_D(const ActionItem);
        return d->commandCheckedDesc;
    }

    void ActionItem::setCommandCheckedDescription(const QPair<QString, QString> &commandCheckedDescription) {
        Q_D(ActionItem);
        d->commandCheckedDesc = commandCheckedDescription;
    }

    bool ActionItem::autoDeleteGlobal() {
        return m_autoDelete;
    }

    void ActionItem::setAutoDeleteGlobal(bool autoDelete) {
        m_autoDelete = autoDelete;
    }

    ActionItem::ActionItem(ActionItemPrivate &d, const QString &id, QObject *parent) : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.id = id;

        d.init();
    }

}