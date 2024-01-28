//
// Created by fluty on 2024/1/23.
//

#include "PianoRollGraphicsView.h"

#include "NoteGraphicsItem.h"
#include "PianoRollBackgroundGraphicsItem.h"
#include "PianoRollGlobal.h"
#include "PianoRollGraphicsScene.h"

PianoRollGraphicsView::PianoRollGraphicsView() {
    setScaleXMax(5);
    setDragMode(RubberBandDrag);

    auto pianoRollScene = new PianoRollGraphicsScene;
    setScene(pianoRollScene);
    connect(this, &PianoRollGraphicsView::scaleChanged, pianoRollScene, &PianoRollGraphicsScene::setScale);

    auto gridItem = new PianoRollBackgroundGraphicsItem;
    gridItem->setPixelsPerQuarterNote(PianoRollGlobal::pixelsPerQuarterNote);
    connect(this, &PianoRollGraphicsView::visibleRectChanged, gridItem, &TimeGridGraphicsItem::setVisibleRect);
    connect(this, &PianoRollGraphicsView::scaleChanged, gridItem, &PianoRollBackgroundGraphicsItem::setScale);
    pianoRollScene->addItem(gridItem);
}
void PianoRollGraphicsView::updateView(const DsModel &model) {
    // TODO: Remove exist note item from scene
    auto pianoScene = dynamic_cast<PianoRollGraphicsScene *>(scene());
    if (pianoScene == nullptr)
        return;

    auto clip = model.tracks.first().clips.first().dynamicCast<DsSingingClip>();
    int noteId = 0;
    for (const auto &note : clip->notes) {
        auto noteItem = new NoteGraphicsItem(noteId);
        noteItem->setStart(note.start());
        noteItem->setLength(note.length());
        noteItem->setKeyIndex(note.keyIndex());
        noteItem->setLyric(note.lyric());
        noteItem->setVisibleRect(this->visibleRect());
        noteItem->setScaleX(this->scaleX());
        noteItem->setScaleY(this->scaleY());
        noteItem->graphicsView = this;
        pianoScene->addItem(noteItem);
        connect(this, &PianoRollGraphicsView::scaleChanged, noteItem,
                         &NoteGraphicsItem::setScale);
        connect(this, &PianoRollGraphicsView::visibleRectChanged, noteItem,
                         &NoteGraphicsItem::setVisibleRect);
        if (noteId == 0)
            this->centerOn(noteItem);
        noteId++;
    }
}