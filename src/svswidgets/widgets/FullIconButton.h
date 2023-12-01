#ifndef FULLICONBUTTON_H
#define FULLICONBUTTON_H

#include <QMWidgets/CPushButton.h>
#include <QMWidgets/QMWidgetsMacros.h>

#include <SVSWidgets/CkSVSWidgetsGlobal.h>

namespace SVS {

    class CKSVSWIDGETS_API FullIconButton : public CPushButton {
        Q_OBJECT
        Q_PROPERTY_DECLARE(QSize, iconMargin, IconMargin)
    public:
        explicit FullIconButton(QWidget *parent = nullptr);
        ~FullIconButton();

    protected:
        void paintEvent(QPaintEvent *event) override;
    };

}

#endif // FULLICONBUTTON_H
