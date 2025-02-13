
#pragma once

#include <QString>
#include <qjsonstream.h>

struct SlurCutterCfg {
    QString open;
    QString next;
    QString prev;
    QString play;

    bool snapToKeys;
    bool showPitchTextOverlay;
    bool showPhonemeTexts;

    SlurCutterCfg() : snapToKeys(true), showPitchTextOverlay(false), showPhonemeTexts(true) {}
};
QAS_JSON_NS(SlurCutterCfg);