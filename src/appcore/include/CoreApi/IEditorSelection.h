#ifndef IEDITORSELECTION_H
#define IEDITORSELECTION_H

#include <QObject>

#include "CkAppCoreGlobal.h"

namespace Core {

    class CKAPPCORE_API IEditorSelection : public QObject {
        Q_OBJECT
    public:
        explicit IEditorSelection(QObject *parent = nullptr);
        ~IEditorSelection();

        enum Feature {
            Cut,
            Copy,
            Paste,
            Del,
            SelectAll,
            Deselect,
        };
        Q_ENUM(Feature)
        Q_DECLARE_FLAGS(Features, Feature)

        virtual Features features() const =  0;

    public:
        virtual void execute(Feature feature);
        virtual bool hasSelection() const;

    signals:
        void changed();
    };

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Core::IEditorSelection::Features)

#endif // IEDITORSELECTION_H