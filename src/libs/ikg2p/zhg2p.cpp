#include "zhg2p.h"
#include "g2pglobal.h"
#include "zhg2p_p.h"

#include <QDebug>
#include <utility>

namespace IKg2p {
    static const QMap<QString, QString> numMap = {
        {"0", "零"},
        {"1", "一"},
        {"2", "二"},
        {"3", "三"},
        {"4", "四"},
        {"5", "五"},
        {"6", "六"},
        {"7", "七"},
        {"8", "八"},
        {"9", "九"}
    };
    // reset pinyin to raw string
    static QString resetZH(const QStringList &input, const QStringList &res, QList<int> &positions) {
        QStringList result = input;
        for (int i = 0; i < positions.size(); i++) {
            result.replace(positions[i], res.at(i));
        }

        return result.join(" ");
    }

    // split the value of pinyin dictionary
    static void addString(const QString &text, QStringList &res) {
        QStringList temp = text.split(" ");
        for (auto &pinyin : qAsConst(temp)) {
            res.append(pinyin);
        }
    }

    // delete elements from the list
    static inline void removeElements(QStringList &list, int start, int n) {
        list.erase(list.begin() + start, list.begin() + start + n);
    }

    ZhG2pPrivate::ZhG2pPrivate(QString language) : m_language(std::move(language)) {
    }

    ZhG2pPrivate::~ZhG2pPrivate() {
    }

    // load zh convert dict
    void ZhG2pPrivate::init() {
        QString dict_dir;
        if (m_language == "mandarin") {
            dict_dir = dictionaryPath() + "/mandarin";
        } else {
            dict_dir = dictionaryPath() + "/cantonese";
        }

        loadDict(dict_dir, "phrases_map.txt", phrases_map);
        loadDict(dict_dir, "phrases_dict.txt", phrases_dict);
        loadDict(dict_dir, "user_dict.txt", phrases_dict);
        loadDict(dict_dir, "word.txt", word_dict);
        loadDict(dict_dir, "trans_word.txt", trans_dict);
    }

    bool ZhG2pPrivate::isPolyphonic(const QString &text) const {
        return phrases_map.contains(text);
    }

    QString ZhG2pPrivate::tradToSim(const QString &text) const {
        return trans_dict.value(text, text);
    }

    QString ZhG2pPrivate::getDefaultPinyin(const QString &text) const {
        return word_dict.value(text, {});
    }

    // get all chinese characters and positions in the list
    void ZhG2pPrivate::zhPosition(const QStringList &input, QStringList &res, QList<int> &positions,
                                  bool covertNum) const {
        for (int i = 0; i < input.size(); i++) {
            if (word_dict.find(input.at(i)) != word_dict.end() || trans_dict.find(input.at(i)) != trans_dict.end()) {
                res.append(input.mid(i, 1));
                positions.append(i);
            } else if (covertNum && numMap.find(input.at(i)) != numMap.end()) {
                res.append(numMap[input.at(i)]);
                positions.append(i);
            }
        }
    }


    ZhG2p::ZhG2p(QString language, QObject *parent) : ZhG2p(*new ZhG2pPrivate(std::move(language)), parent) {
    }

    ZhG2p::~ZhG2p() {
    }

    QString ZhG2p::convert(const QString &input, bool tone, bool covertNum) {
        return convert(splitString(input), tone, covertNum);
    }

    QString ZhG2p::convert(const QStringList &input, bool tone, bool covertNum) {
        Q_D(const ZhG2p);
        //    qDebug() << input;
        QStringList inputList;
        QList<int> inputPos;
        // get char&pos in dict
        d->zhPosition(input, inputList, inputPos, covertNum);
        QStringView cleanInput = QStringView(inputList.join(""));
        QStringList result;
        int cursor = 0;
        while (cursor < inputList.size()) {
            // get char
            const QString &raw_current_char = inputList.at(cursor);
            QString current_char;
            current_char = d->tradToSim(raw_current_char);

            // not in dict
            if (d->word_dict.find(current_char) == d->word_dict.end()) {
                result.append(current_char);
                cursor++;
                continue;
            }

            //        qDebug() << current_char << isPolyphonic(current_char);
            // is polypropylene
            if (!d->isPolyphonic(current_char)) {
                result.append(d->getDefaultPinyin(current_char));
                cursor++;
            } else {
                bool found = false;
                for (int length = 4; length >= 2 && !found; length--) {
                    if (cursor + length <= inputList.size()) {
                        // cursor: 地, subPhrase: 地久天长
                        QStringView sub_phrase = cleanInput.mid(cursor, length);
                        if (d->phrases_dict.find(sub_phrase) != d->phrases_dict.end()) {
                            result.append(d->phrases_dict[sub_phrase]);
                            cursor += length;
                            found = true;
                        }

                        if (cursor >= 1 && !found) {
                            // cursor: 重, subPhrase_1: 语重心长
                            QStringView sub_phrase_1 = cleanInput.mid(cursor - 1, length);
                            if (d->phrases_dict.find(sub_phrase_1) != d->phrases_dict.end()) {
                                result.removeAt(result.size() - 1);
                                result.append(d->phrases_dict[sub_phrase_1]);
                                cursor += length - 1;
                                found = true;
                            }
                        }
                    }

                    if (cursor + 1 >= length && !found && cursor + 1 <= inputList.size()) {
                        // cursor: 好, xSubPhrase: 各有所好
                        QStringView x_sub_phrase = cleanInput.mid(cursor + 1 - length, length);
                        if (d->phrases_dict.find(x_sub_phrase) != d->phrases_dict.end()) {
                            // overwrite pinyin
                            removeElements(result, cursor + 1 - length, length - 1);
                            result.append(d->phrases_dict[x_sub_phrase]);
                            cursor += 1;
                            found = true;
                        }
                    }

                    if (cursor + 2 >= length && !found && cursor + 2 <= inputList.size()) {
                        // cursor: 好, xSubPhrase: 叶公好龙
                        QStringView x_sub_phrase_1 = cleanInput.mid(cursor + 2 - length, length);
                        if (d->phrases_dict.find(x_sub_phrase_1) != d->phrases_dict.end()) {
                            // overwrite pinyin
                            removeElements(result, cursor + 2 - length, length - 2);
                            result.append(d->phrases_dict[x_sub_phrase_1]);
                            cursor += 2;
                            found = true;
                        }
                    }
                }

                // not found, use default pinyin
                if (!found) {
                    result.append(d->getDefaultPinyin(current_char));
                    cursor++;
                }
            }
        }

        if (!tone) {
            for (QString &item : result) {
                if (item[item.size() - 1].isDigit()) {
                    item.remove(item.size() - 1, 1);
                }
            }
        }

        return resetZH(input, result, inputPos);
    }

    ZhG2p::ZhG2p(ZhG2pPrivate &d, QObject *parent) : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;

        d.init();
    }

}
