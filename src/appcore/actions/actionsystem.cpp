#include "actionsystem.h"
#include "actionsystem_p.h"

#include <QDebug>
#include <QFile>

#include <QMCore/qmbatch.h>

#include "iloader.h"

namespace Core {

#define myWarning(func) (qWarning().nospace() << "Core::ActionSystem::" << (func) << "():").space()

    ActionSystemPrivate::ActionSystemPrivate() : q_ptr(nullptr) {
    }

    ActionSystemPrivate::~ActionSystemPrivate() = default;

    void ActionSystemPrivate::init() {
    }

    static ActionSystem *m_instance = nullptr;

    ActionSystem::ActionSystem(QObject *parent) : ActionSystem(*new ActionSystemPrivate(), parent) {
    }

    ActionSystem::~ActionSystem() {
        Q_D(ActionSystem);
        m_instance = nullptr;
    }

    // Recognize semicolon as the delimiter, you should use two consecutive semicolon to unescape
    static QList<QKeySequence> parseShortcuts(const QString &text) {
        QString curText;
        QList<QKeySequence> res;
        for (auto it = text.begin(); it != text.end(); ++it) {
            const auto &ch = *it;
            if (ch == ';') {
                if (it != text.end() && *(it + 1) == ';') {
                    it++;
                    curText += ";";
                } else {
                    res.append(QKeySequence(curText));
                    curText.clear();
                }
            } else {
                curText += ch;
            }
        }
        if (!curText.isEmpty()) {
            res.append(QKeySequence(curText));
        }
        return res;
    }

    ActionSystem::ActionSystem(ActionSystemPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        m_instance = this;

        d.q_ptr = this;
        d.init();
    }

}