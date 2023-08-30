#ifndef CHORUSKIT_TITLELISTWIDGET_P_H
#define CHORUSKIT_TITLELISTWIDGET_P_H

#include "TitleListItemDelegate.h"
#include "TitleListWidget.h"

namespace SVS {

    class TitleListWidgetPrivate : public QObject {
        Q_OBJECT
        Q_DECLARE_PUBLIC(TitleListWidget)
    public:
        TitleListWidgetPrivate();
        ~TitleListWidgetPrivate();

        void init();

        TitleListWidget *q_ptr;

        TitleListItemDelegate *m_delegate;
        QSize oldContentsSize;

    private:
        void _q_delegateClicked(const QModelIndex &index, int button);
    };

}

#endif // CHORUSKIT_TITLELISTWIDGET_P_H
