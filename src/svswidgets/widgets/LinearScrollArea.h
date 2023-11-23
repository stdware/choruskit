#ifndef LINEARSCROLLAREA_H
#define LINEARSCROLLAREA_H

#include <QScrollArea>

#include "CkSVSWidgetsGlobal.h"

namespace SVS {

    class CKSVSWIDGETS_API LinearScrollArea : public QScrollArea {
        Q_OBJECT
    public:
        explicit LinearScrollArea(Qt::Orientation ori, QWidget *parent = nullptr);
        ~LinearScrollArea();

    public:
        Qt::Orientation orient() const;

    protected:
        Qt::Orientation m_orient;
        
        void resizeEvent(QResizeEvent *event) override;
    };

}

#endif // LINEARSCROLLAREA_H
