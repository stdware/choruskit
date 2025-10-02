#include "splashconfig.h"

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

namespace Loader {

    bool SplashConfig::load(const QString &filename) {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }
        QByteArray data(file.readAll());
        file.close();

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            return false;
        }

        auto objDoc = doc.object();

        // splashImage
        auto it = objDoc.find("splashImage");
        if (it != objDoc.end() && it.value().isString()) {
            splashImage = it.value().toString();
        }

        // splashSize
        it = objDoc.find("splashSize");
        if (it != objDoc.end() && it.value().isArray()) {
            const QJsonArray &sizeArr = it.value().toArray();
            for (const QJsonValue &val : sizeArr) {
                splashSize.append(val.toInt());
            }
        }

        // splashSettings
        it = objDoc.find("splashSettings");
        if (it != objDoc.end() && it.value().isObject()) {
            const QJsonObject &settingsObj = it.value().toObject();

            // splashSettings.size
            auto itSize = settingsObj.find("size");
            if (itSize != settingsObj.end() && itSize.value().isArray()) {
                const QJsonArray &sizeArr = itSize.value().toArray();
                for (const QJsonValue &val : sizeArr) {
                    splashSettings.size.append(val.toInt());
                }
            }

            // splashSettings.texts
            auto itTexts = settingsObj.find("texts");
            if (itTexts != settingsObj.end() && itTexts.value().isObject()) {
                const QJsonObject &textsObj = itTexts.value().toObject();

                for (auto keyIt = textsObj.begin(); keyIt != textsObj.end(); ++keyIt) {
                    const QString &key = keyIt.key();
                    const QJsonValue &value = keyIt.value();

                    if (!value.isObject())
                        continue;

                    const QJsonObject &textObj = value.toObject();
                    SplashText splashText;

                    // pos
                    auto itPos = textObj.find("pos");
                    if (itPos != textObj.end() && itPos.value().isArray()) {
                        for (const QJsonValue &val : itPos.value().toArray()) {
                            splashText.pos.append(val.toInt());
                        }
                    }

                    // alignment
                    auto itAlignment = textObj.find("alignment");
                    if (itAlignment != textObj.end() && itAlignment.value().isDouble()) {
                        splashText.alignment = itAlignment.value().toInt();
                    }

                    // fontSize
                    auto itFontSize = textObj.find("fontSize");
                    if (itFontSize != textObj.end() && itFontSize.value().isDouble()) {
                        splashText.fontSize = itFontSize.value().toInt();
                    }

                    // fontColor
                    auto itFontColor = textObj.find("fontColor");
                    if (itFontColor != textObj.end() && itFontColor.value().isString()) {
                        splashText.fontColor = itFontColor.value().toString();
                    }

                    // maxWidth
                    auto itMaxWidth = textObj.find("maxWidth");
                    if (itMaxWidth != textObj.end() && itMaxWidth.value().isDouble()) {
                        splashText.maxWidth = itMaxWidth.value().toInt();
                    }

                    // text
                    auto itText = textObj.find("text");
                    if (itText != textObj.end() && itText.value().isString()) {
                        splashText.text = itText.value().toString();
                    }

                    splashSettings.texts.insert(key, splashText);
                }
            }
        }
        return true;
    }

}
