#include "SplashScreen.h"

#include <utility>

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtGui/QPainter>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QScreen>

#include "loaderutils.h"

#include "splashconfig.h"

namespace Loader {

    static QImage generateTextImage(const QString &text, int width = 800, int height = 200) {
        QImage image(width, height, QImage::Format_ARGB32);
        image.fill(Qt::gray);

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);

        QFont font = painter.font();
        font.setPixelSize(100);
        painter.setFont(font);
        painter.setPen(Qt::white);
        QRect textRect = painter.boundingRect(image.rect(), Qt::AlignCenter, text);
        painter.drawText(textRect, Qt::AlignCenter, text);
        painter.end();

        return image;
    }

    SplashScreen::SplashScreen(QScreen *screen) : QSplashScreen(screen) {
        m_showTexts = false;
        m_texts.insert("_status", {});

        connect(this, &QSplashScreen::messageChanged, this, &SplashScreen::_q_messageChanged);
    }

    SplashScreen::~SplashScreen() {
    }

    void SplashScreen::applyConfig(const QString &fileName) {
        QString splashImagePath;
        QImage splashImage;
        QSize splashSize;

        QString configDir = QFileInfo(fileName).absolutePath();

        // Load configuration
        SplashConfig configFile;
        if (configFile.load(fileName)) {
            if (!configFile.splashImage.isEmpty()) {
                QString path = configFile.splashImage;
                if (QDir::isRelativePath(path)) {
                    path = configDir + "/" + path;
                }
                splashImagePath = path;
            }
            if (configFile.splashSize.size() == 2) {
                splashSize = QSize(configFile.splashSize.front(), configFile.splashSize.back());
            }
        }

        splashImage = QImage(splashImagePath);

        if (splashImage.isNull()) {
            splashImage = generateTextImage(qApp->applicationDisplayName());
            splashSize = splashImage.size();
        }

        if (splashSize.isEmpty()) {
            splashSize = splashImage.size();
        }

        // Setup splash
        QPixmap pixmap;
        if (splashImagePath.endsWith(".svg", Qt::CaseInsensitive)) {
            pixmap = QIcon(splashImagePath).pixmap(splashSize * screen()->devicePixelRatio());
        } else {
            pixmap = QPixmap::fromImage(
                splashImage.scaled(splashSize * screen()->devicePixelRatio(), Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation));
        }
        pixmap.setDevicePixelRatio(screen()->devicePixelRatio());
        setPixmap(pixmap);

        for (auto it = configFile.splashSettings.texts.begin();
             it != configFile.splashSettings.texts.end(); ++it) {
            const auto &item = it.value();
            SplashScreen::Attribute attr;
            attr.pos = item.pos.size() == 2 ? QPoint(item.pos[0], item.pos[1]) : attr.pos;
            attr.anchor =
                item.anchor.size() == 2 ? qMakePair(item.anchor[0], item.anchor[1]) : attr.anchor;
            attr.fontSize = item.fontSize > 0 ? item.fontSize : attr.fontSize;
            attr.fontColor = Loader::parseColor(item.fontColor);
            attr.maxWidth = item.maxWidth > 0 ? item.maxWidth : attr.maxWidth;
            attr.text = item.text;
            setTextAttribute(it.key(), attr);
        }
    }

    void SplashScreen::setTextAttribute(const QString &id, const SplashScreen::Attribute &attr) {
        m_texts[id] = attr;

        if (m_showTexts && isVisible())
            repaint();
    }

    void SplashScreen::setText(const QString &id, const QString &text) {
        auto it = m_texts.find(id);
        if (it == m_texts.end())
            return;
        it->text = text;

        if (m_showTexts && isVisible())
            repaint();
    }

    void SplashScreen::drawContents(QPainter *painter) {
        // QSplashScreen::drawContents(painter);

        if (!m_showTexts) {
            return;
        }

        // Draw texts
        for (const auto &item : std::as_const(m_texts)) {
            const Attribute &attr = item;

            QFont font = Loader::systemDefaultFont();
            font.setPixelSize(attr.fontSize);
            QFontMetrics fm(font);

            QPoint pos(attr.pos);
            if (pos.x() < 0) {
                pos.rx() += this->width();
            }
            if (pos.y() < 0) {
                pos.ry() += this->height();
            }

            int w = this->width();
            int maxWidth = qMin(attr.anchor.first > 0 ? w - pos.x() : pos.x(),
                                attr.maxWidth > 0 ? attr.maxWidth : w);
            QRect dst(attr.anchor.first > 0 ? pos.x() : pos.x() - maxWidth,
                      attr.anchor.second > 0 ? pos.y() : pos.y() - fm.height(), maxWidth,
                      fm.height());

            QString text = fm.horizontalAdvance(item.text) > dst.width()
                               ? fm.elidedText(item.text, Qt::ElideRight, dst.width())
                               : item.text;
            painter->setPen(QPen(attr.fontColor));
            painter->setFont(font);
            painter->drawText(
                dst, (attr.anchor.first > 0 ? Qt::AlignLeft : Qt::AlignRight) | Qt::AlignVCenter,
                text);
        }
    }

    void SplashScreen::showTexts() {
        m_showTexts = true;
        if (isVisible())
            repaint();
    }

    void SplashScreen::mousePressEvent(QMouseEvent *event) {
        // No hide
    }

    void SplashScreen::_q_messageChanged(const QString &message) {
        setText("_status", message);
    }

    void SplashScreen::closeEvent(QCloseEvent *event) {
        Q_EMIT closed();
    }

}
