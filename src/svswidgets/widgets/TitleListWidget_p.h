#ifndef TITLELISTWIDGET_P_H
#define TITLELISTWIDGET_P_H

#include <SVSWidgets/TitleListItemDelegate.h>
#include <SVSWidgets/TitleListWidget.h>

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

#endif // TITLELISTWIDGET_P_H
