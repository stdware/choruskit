#ifndef TITLELISTITEMDELEGATE_H
#define TITLELISTITEMDELEGATE_H

#include <QStyledItemDelegate>

#include <QMGui/QCssValueMap.h>
#include <SVSWidgets/CkSVSWidgetsGlobal.h>

namespace SVS {

    /**
     *
     * StyleData:
     *  - background: QRectStyle
     *  - underline: QPen
     *  - titleShape: QTypeFace
     *  - subShape: QTypeFace
     *  - descShape: QTypeFace
     *  - descHighlightShape: QTypeFace
     *  - descBackgroundShape: QTypeFace
     *  - titleMargins: QMargins
     *  - subMargins: QMargins
     *  - descMargins: QMargins
     *  - iconMargins: QMargins
     *  - padding: QMargins
     *  - margins: QMargins
     *  - defaultIconSize: QSize
     *
     */

    class TitleListItemDelegatePrivate;

    class CKSVSWIDGETS_API TitleListItemDelegate : public QStyledItemDelegate {
        Q_OBJECT
        Q_DECLARE_PRIVATE(TitleListItemDelegate)
    public:
        explicit TitleListItemDelegate(QObject *parent = nullptr);
        ~TitleListItemDelegate();

    public:
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    public:
        QCssValueMap styleData() const;
        void setStyleData(const QCssValueMap &map);

    signals:
        void clicked(const QModelIndex &index, int button);

    protected:
        bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                         const QModelIndex &index) override;

    protected:
        TitleListItemDelegate(TitleListItemDelegatePrivate &d, QObject *parent = nullptr);

        QScopedPointer<TitleListItemDelegatePrivate> d_ptr;
    };

}

#endif // TITLELISTITEMDELEGATE_H