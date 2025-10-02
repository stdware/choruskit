#ifndef CHORUSKIT_SPLASHSCREEN_H
#define CHORUSKIT_SPLASHSCREEN_H

#include <QSplashScreen>

namespace Loader {

    class SplashScreen : public QSplashScreen {
        Q_OBJECT
    public:
        explicit SplashScreen(QScreen *screen = nullptr);
        ~SplashScreen();

        struct Attribute {
            QPoint pos;
            Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignTop;
            int fontSize = 12;
            QColor fontColor;
            int maxWidth = 0;
            QString text;
        };

        void applyConfig(const QString &fileName);

    public:
        void setTextAttribute(const QString &id, const Attribute &attr);
        Q_INVOKABLE void setText(const QString &id, const QString &text);

        // The first display of text takes a relatively long time,
        // So we firstly display the image background, then show texts
        void showTexts();

    Q_SIGNALS:
        void closed();

    protected:
        void drawContents(QPainter *painter) override;

        void mousePressEvent(QMouseEvent *event) override;
        void closeEvent(QCloseEvent *event) override;

    private:
        bool m_showTexts;
        QHash<QString, Attribute> m_texts;

        void _q_messageChanged(const QString &message);
    };

}

#endif // CHORUSKIT_SPLASHSCREEN_H
