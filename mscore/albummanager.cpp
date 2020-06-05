//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <iostream>

#include "libmscore/album.h"
#include "albummanager.h"
#include "albummanagerdialog.h"
#include "seq.h"
#include "globals.h"
#include "musescore.h"
#include "scoreview.h"
#include "preferences.h"
#include "icons.h"
#include "libmscore/mscore.h"
#include "libmscore/xml.h"
#include "libmscore/undo.h"
#include "scoretab.h"

using namespace std;

namespace Ms {
//---------------------------------------------------------
//---------------------------------------------------------
//
//   AlbumManager
//
//---------------------------------------------------------
//---------------------------------------------------------

AlbumManager::AlbumManager(QWidget* parent)
    : QDockWidget(parent)
{
    // window
    setObjectName("AlbumManager");
    setupUi(this);
    setWindowFlags(Qt::Tool);   // copy paste from play panel
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));   // copy paste from play panel
    mscore->addDockWidget(Qt::RightDockWidgetArea, this);

    // buttons
    up->setIcon(*icons[int(Icons::arrowUp_ICON)]);
    down->setIcon(*icons[int(Icons::arrowDown_ICON)]);

    connect(add,                &QPushButton::clicked,  this, &AlbumManager::addClicked);
    connect(addNew,             &QPushButton::clicked,  this, &AlbumManager::addNewClicked);
    connect(up,                 &QPushButton::clicked,  this, &AlbumManager::upClicked);
    connect(down,               &QPushButton::clicked,  this, &AlbumManager::downClicked);
    connect(remove,             &QPushButton::clicked,  this, &AlbumManager::removeClicked);
    connect(deleteButton,       &QPushButton::clicked,  this, &AlbumManager::deleteClicked);
    connect(albumModeButton,    &QRadioButton::toggled, this, &AlbumManager::changeMode);
    connect(scoreModeButton,    &QRadioButton::toggled, this, &AlbumManager::changeMode);
    connect(settingsButton,     &QPushButton::clicked,  this, &AlbumManager::openSettingsDialog);
    connect(playButton,         &QPushButton::clicked,  this, static_cast<void (AlbumManager::*)(bool)>(&AlbumManager::playAlbum));
    connect(scoreList,          &QTableWidget::itemChanged, this,             &AlbumManager::itemChanged);
    connect(scoreList,          &QTableWidget::itemDoubleClicked, this,       &AlbumManager::itemDoubleClicked);
    connect(scoreList,          &QTableWidget::itemSelectionChanged, this,    &AlbumManager::updateButtons);
    updateButtons();
    add->setEnabled(true);

    // drag & drop
    scoreList->setDragEnabled(true);
    scoreList->setAcceptDrops(true);
    scoreList->horizontalHeader()->setSectionsMovable(true);
    scoreList->viewport()->installEventFilter(this);
    connect(scoreList->model(), &QAbstractItemModel::rowsMoved, this, &AlbumManager::updateScoreOrder);

    // the rest
    updateDurations();
    MuseScore::restoreGeometry(this);
}

AlbumManager::~AlbumManager()
{
    if (isVisible()) {
        MuseScore::saveGeometry(this);
    }
}

//---------------------------------------------------------
//   eventFilter
///    Used to handle drag & drop.
//---------------------------------------------------------

bool AlbumManager::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == scoreList->viewport() && ev->type() == QEvent::DragEnter) {
        QDragEnterEvent* dEvent = static_cast<QDragEnterEvent*>(ev);
        QPoint pos = dEvent->pos();
        dragEnterIndex = scoreList->rowAt(pos.y());
    }
    if (obj == scoreList->viewport() && ev->type() == QEvent::Drop) {
        QDropEvent* dEvent = static_cast<QDropEvent*>(ev);
        QPoint pos = dEvent->pos();
        dropIndex = scoreList->rowAt(pos.y());
        if (dropIndex > dragEnterIndex) {
            for (int i = dragEnterIndex; i < dropIndex; i++) {
                swap(i, i + 1);
            }
        } else if (dropIndex < dragEnterIndex) {
            for (int i = dragEnterIndex; i > dropIndex; i--) {
                swap(i, i - 1);
            }
        }
        scoreList->setCurrentCell(dropIndex, scoreList->currentColumn());
        ev->ignore();
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void AlbumManager::hideEvent(QHideEvent* event)
{
    MuseScore::saveGeometry(this);
    QDockWidget::hideEvent(event);
}

//---------------------------------------------------------
//   changeMode
///   Change between score mode and album mode.
//---------------------------------------------------------

void AlbumManager::changeMode(bool throwaway)
{
    if (scoreModeButton->isChecked()) {
        albumModeButton->setChecked(false);
        mscore->closeScore(tempScore); // also frees
        tempScore = nullptr;
    } else if (albumModeButton->isChecked()) {
        scoreModeButton->setChecked(false);
        if (!tempScore) {
            tempScore = _items.at(0)->albumItem->score->clone(); // TODO: clone breaks editing sync for the 1st movement
            mscore->setCurrentScoreView(mscore->appendScore(tempScore));
            mscore->getTab1()->setTabText(mscore->getTab1()->count() - 1, "Temporary Album Score");
            for (auto item : _items) {
                if (item == _items.at(0)) {
                    continue;
                }
                cout << "adding score: " << item->albumItem->score->title().toStdString() << endl;
                tempScore->addMovement(item->albumItem->score);
            }
            tempScore->setLayoutAll();
            tempScore->update();

//            _album->addScore(tempScore);
//            addAlbumItem(_album->_albumItems.back());
        }
    } else {
        Q_ASSERT(false);
    }
}

//---------------------------------------------------------
//   playAlbum
//---------------------------------------------------------

void AlbumManager::playAlbum()
{
    static int i { -1 };
    i++;

    if (i < _items.size()) {
        if (_items.at(i)->albumItem->enabled) {
            if (scoreModeButton->isChecked()) {
                if (_items.at(i)->albumItem->score) {
                    mscore->openScore(_items.at(i)->albumItem->_fileInfo.absoluteFilePath());         // what if the files have not been saved?
                } else {
                    _items.at(i)->albumItem->score = mscore->openScore(_items.at(i)->albumItem->_fileInfo.absoluteFilePath());
                }
                mscore->currentScoreView()->gotoMeasure(_items.at(i)->albumItem->score->firstMeasure());       // rewind before playing
            }

            if (i == 0) {
                seq->start();
            } else {
                QTimer::singleShot(_album->playbackDelay, this, &AlbumManager::startPlayback);
            }
            if (i == 0) {
                connect(seq, &Seq::stopped, this, static_cast<void (AlbumManager::*)()>(&AlbumManager::playAlbum));
            }
        } else {
            if (albumModeButton->isChecked()) {
                seq->setNextScore();
            }
            playAlbum();
        }
    } else {
        i = -1;
        disconnect(seq, &Seq::stopped, this, static_cast<void (AlbumManager::*)()>(&AlbumManager::playAlbum));
    }
}

void AlbumManager::playAlbum(bool throwaway)
{
    playAlbum();
}

//---------------------------------------------------------
//   startPlayback
//---------------------------------------------------------

void AlbumManager::startPlayback()
{
    seq->start();
}

//---------------------------------------------------------
//   updateScoreOrder
///    Called when the scoreList/View is reordered.\n
///    (e.g. Drag&Drop)
//---------------------------------------------------------

void AlbumManager::updateScoreOrder(QModelIndex sourceParent, int sourceStart, int sourceEnd,
                                    QModelIndex destinationParent, int destinationRow)
{
    for (int i = 0; i < _items.size(); i++) {
        for (int j = 0; j < _items.size(); j++) {
            if (_items.at(j)->albumItem->score->title() != scoreList->item(j, 0)->text()) {
                int h = scoreList->row(scoreList->findItems(_items.at(j)->albumItem->score->title(),
                                                            Qt::MatchExactly).first());
                std::swap(_items.at(j), _items.at(h));
                break;
            } else if (j == _items.size() - 1) {
                goto end;
            }
        }
    }
end:;
    updateButtons();
}

//---------------------------------------------------------
//   openSettingsDialog
///   (Re)Open the settings dialog menu.
//---------------------------------------------------------

void AlbumManager::openSettingsDialog(bool throwaway)
{
    if (!settingsDialog) {
        settingsDialog = new AlbumManagerDialog(this);
        settingsDialog->start();
    } else {
        settingsDialog->start();
    }
}


//---------------------------------------------------------
//   albumScores
///     this is a leftover before the refactor
//---------------------------------------------------------

std::vector<AlbumItem*> AlbumManager::albumScores() const
{
    return _album->_albumItems;
}

//---------------------------------------------------------
//   getScoreTitle
///   TODO: What does this do and why?
//---------------------------------------------------------

static QString getScoreTitle(Score* score)
{
    QString name = score->metaTag("movementTitle");
    if (name.isEmpty()) {
        Text* t = score->getText(Tid::TITLE);
        if (t) {
            name = QTextDocumentFragment::fromHtml(t->xmlText()).toPlainText().replace("&amp;","&")
                    .replace("&gt;",">").replace("&lt;","<").replace("&quot;", "\"");
        }
        name = name.simplified();
    }
    if (name.isEmpty()) {
        name = score->title();
    }
    return name;
}

//---------------------------------------------------------
//   addClicked
///   Add an existing score to the Album.\n
///   Opens a dialog to select a Score from the filesystem.
//---------------------------------------------------------

void AlbumManager::addClicked(bool throwaway)
{
    QStringList files = mscore->getOpenScoreNames(
        tr("MuseScore Files") + " (*.mscz *.mscx);;", tr("Load Score")
        );
    QList<MasterScore*> scores;
    for (const QString& fn : files) {
        MasterScore* score = mscore->readScore(fn);
        Movements* m = score->movements();
        for (MasterScore* ms : *m) {
            scores.push_back(ms);
            ms->setMovements(0);
        }
        delete m;
    }
    if (scores.empty()) {
        return;
    }
    for (MasterScore* score : scores) {
        _album->addScore(score);
        addAlbumItem(_album->_albumItems.back());
    }
}

//---------------------------------------------------------
//   addNewClicked
///   Add a new Score to the Album.
//---------------------------------------------------------

void AlbumManager::addNewClicked(bool throwaway)
{
    MasterScore* score = mscore->getNewFile();
    if (!score) {
        return;
    }
    _album->addScore(score);
    addAlbumItem(_album->_albumItems.back());
}

//---------------------------------------------------------
//   addAlbumItem
///   add the given AlbumItem to the AlbumManager
///   creates the corresponding scoreList/View item
///   the AlbumItem and Widget are saved in a new AlbumManagerItem
//---------------------------------------------------------

void AlbumManager::addAlbumItem(AlbumItem *albumItem)
{
    if (!albumItem) {
        std::cout << "empty album item" << std::endl;
        return;
    }
    scoreList->blockSignals(true);
    QString name = albumItem->score->title();
    QTableWidgetItem* li = new QTableWidgetItem(name);
    scoreList->setRowCount(scoreList->rowCount() + 1);
    scoreList->setItem(scoreList->rowCount() - 1, 0, li);
    li->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable));
    li->setCheckState(Qt::CheckState::Checked);
    QTableWidgetItem* tid = new QTableWidgetItem("00:00:00");
    scoreList->setItem(scoreList->rowCount() - 1, 1, tid);
    tid->setFlags(Qt::ItemFlags(Qt::ItemIsEnabled));
    AlbumManagerItem* albumManagerItem = new AlbumManagerItem(albumItem, li, tid);
    _items.push_back(albumManagerItem);
    scoreList->blockSignals(false);
    updateDurations();
}

//---------------------------------------------------------
//   updateDurations
///   Calculates and updates (the labels of) the duration
///   of the Album and of each individual score.
//---------------------------------------------------------

void AlbumManager::updateDurations()
{
    scoreList->blockSignals(true);
    int seconds = 0;
    int minutes = 0;
    int hours = 0;

    for (auto item : _items) {
        bool temporarilyOpen = false;
        if (item->albumItem->score == nullptr) {
            item->albumItem->setScore(mscore->openScore(item->albumItem->_fileInfo.absoluteFilePath(), false));
            temporarilyOpen = true;
        }

        if (item->albumItem->enabled) {
            seconds += item->albumItem->score->duration();
        }

        int tempSeconds = item->albumItem->score->duration();
        int tempMinutes = tempSeconds / 60;
        tempSeconds -= tempMinutes * 60;
        int tempHours = tempMinutes / 60;
        tempMinutes -= tempHours * 60;

        item->listDurationItem->setText(
            QString::number(tempHours).rightJustified(2, '0') + ":"
            + QString::number(tempMinutes).rightJustified(2, '0') + ":"
            + QString::number(tempSeconds).rightJustified(2, '0'));

        if (temporarilyOpen) {
            mscore->closeScore(item->albumItem->score);
        }
    }
    minutes = seconds / 60;
    seconds -= minutes * 60;
    hours = minutes / 60;
    minutes -= hours * 60;

    durationLabel->setText(QString::number(hours).rightJustified(2, '0') + ":" + QString::number(minutes).rightJustified(2,
                               '0') + ":" + QString::number(seconds).rightJustified(2, '0'));
    scoreList->blockSignals(false);
}

//---------------------------------------------------------
//   upClicked
///   Up arrow clicked.
//---------------------------------------------------------

void AlbumManager::upClicked(bool throwaway)
{
    int index = scoreList->currentRow();
    if (index == -1 || index == 0) {
        return;
    }
    swap(index, index - 1);
    scoreList->setCurrentCell(index - 1, 0);
}

//---------------------------------------------------------
//   downClicked
///    Down arrow clicked.
//---------------------------------------------------------

void AlbumManager::downClicked(bool throwaway)
{
    int index = scoreList->currentRow();
    if (index == -1 || index == scoreList->rowCount() - 1) {
        return;
    }
    swap(index, index + 1);
    scoreList->setCurrentCell(index + 1, 0);
}

//---------------------------------------------------------
//   itemDoubleClicked
///   Called when one of the Widgets in the scoreList/View
///   gets clicked. \n
///   In Score mode:
///   This either opens the clicked Score or changes to the
///   corresponding tab if it is already open. \n
///   TODO In Album mode:
///   This centers the view to the part of the tempScore
///   where the clicked Score begins.
//---------------------------------------------------------

void AlbumManager::itemDoubleClicked(QTableWidgetItem* item)
{
//      if (scoreModeButton->isChecked()) {
    for (auto& x : _items) {
        if (x->listItem == item) {
            if (x->albumItem->score) {
                mscore->openScore(x->albumItem->_fileInfo.absoluteFilePath());
            } else {
                x->albumItem->score = mscore->openScore(x->albumItem->_fileInfo.absoluteFilePath());
                x->albumItem->score->setPartOfActiveAlbum(true);
            }
            x->albumItem->score->doLayout();
        }
    }
//            }
}

//---------------------------------------------------------
//   swap
///   Swap the 2 given AlbumScores.
//---------------------------------------------------------

void AlbumManager::swap(int indexA, int indexB)
{
    //
    // The problem is that the widgets are contained in both the AlbumScores and their
    // main container, that's why I need to swap them multiple times.
    //
    scoreList->blockSignals(true);
    _album->swap(indexA, indexB);
    // swap them in their container
    std::swap(_items.at(indexA), _items.at(indexB));

    // swap the text of the widgets
    QTableWidgetItem* itemA = scoreList->item(indexA, 0);
    itemA->setText(_items.at(indexA)->albumItem->score->title());
    QTableWidgetItem* itemB = scoreList->item(indexB, 0);
    itemB->setText(_items.at(indexB)->albumItem->score->title());

    // swap again the widgets to place them correctly FIXME: isn't there a better way to do all this?
    std::swap(_items.at(indexA)->listItem, _items.at(indexB)->listItem);   // workaround, because the list widget items are changed twice so they are being reset
    std::swap(_items.at(indexA)->listDurationItem, _items.at(indexB)->listDurationItem);

    // update the enabled indicators
    _items.at(indexA)->setEnabled(_items.at(indexA)->albumItem->enabled);
    _items.at(indexB)->setEnabled(_items.at(indexB)->albumItem->enabled);

    // update the duration labels
    updateDurations();
    scoreList->blockSignals(false);
}

//---------------------------------------------------------
//   removeClicked
//---------------------------------------------------------

void AlbumManager::removeClicked(bool throwaway)
{
    _items.erase(_items.begin() + scoreList->currentRow());
    _album->removeScore(scoreList->currentRow());
    scoreList->removeRow(scoreList->currentRow());
    updateDurations();
}

//---------------------------------------------------------
//   deleteClicked
//---------------------------------------------------------

void AlbumManager::deleteClicked(bool throwaway)
{
}

//---------------------------------------------------------
//   setAlbum
//---------------------------------------------------------

void AlbumManager::setAlbum(Album* a)
{
    std::cout << "setting album" << std::endl;
    scoreList->setRowCount(0);
    _items.clear(); // TODO: also free all
    if (!a)
        return;

    _album = a;

    scoreList->blockSignals(true);
    for (auto& item : _album->_albumItems) {
        mscore->openScore(item->_fileInfo.absoluteFilePath());
        item->setScore(mscore->currentScoreView()->score()->masterScore());
        addAlbumItem(item);
    }
    scoreList->blockSignals(false);
}

//---------------------------------------------------------
//   updateButtons
///   Activates/Deactivates buttons depending on the selected row
///   and whether there are Scores in the Album.
//---------------------------------------------------------

void AlbumManager::updateButtons()
{
    int idx = scoreList->currentRow();
    int n = scoreList->rowCount();
    if (n == 0) {
        up->setEnabled(false);
        down->setEnabled(false);
        remove->setEnabled(false);
        return;
    }
    down->setEnabled(idx < (n - 1));
    up->setEnabled(idx > 0);
    remove->setEnabled(true);
}

//---------------------------------------------------------
//   itemChanged
///   Called when the state of the item changes.
///   Updates the duration and whether is it enabled.
//---------------------------------------------------------

void AlbumManager::itemChanged(QTableWidgetItem* item)
{
    scoreList->blockSignals(true);
    if (item->column() == 0) {
        AlbumManagerItem* albumManagerItem = _items.at(scoreList->row(item));
        if (item->checkState() == Qt::CheckState::Checked) {
            albumManagerItem->setEnabled(true);
        } else {
            albumManagerItem->setEnabled(false);
        }
        updateDurations();
    }
    scoreList->blockSignals(false);
}

//---------------------------------------------------------
//   showAlbumManager
//---------------------------------------------------------

void MuseScore::showAlbumManager()
{
    if (albumManager == 0) {
        albumManager = new AlbumManager(this);
    }

//      if (currentScoreView() && currentScoreView()->score() && currentScoreView()->score()->masterScore()) // add the current score to the album
//            albumManager->addScore(currentScoreView()->score()->masterScore());
    albumManager->show();
    albumManager->albumTitleEdit->setFocus();
}

//---------------------------------------------------------
//---------------------------------------------------------
//
//   AlbumManagerItem
//
//---------------------------------------------------------
//---------------------------------------------------------

AlbumManagerItem::AlbumManagerItem(AlbumItem* item, QTableWidgetItem* listItem, QTableWidgetItem* listDurationItem)
{
    albumItem = item;
    if (!albumItem->score) {
        albumItem->setScore(mscore->openScore(albumItem->_fileInfo.absoluteFilePath()));
    }
    albumItem->score->setPartOfActiveAlbum(true);
    this->listItem = listItem;
    this->listDurationItem = listDurationItem;
}

AlbumManagerItem::~AlbumManagerItem()
{
    albumItem->score->setPartOfActiveAlbum(false);
}

//---------------------------------------------------------
//   setEnabled
//---------------------------------------------------------

void AlbumManagerItem::setEnabled(bool b)
{
    albumItem->enabled = b;
    if (b) {
        if (listItem) {
            listItem->setTextColor(Qt::black);
            listItem->setCheckState(Qt::CheckState::Checked);       // used for initialization
        }
        if (listDurationItem) {
            listDurationItem->setTextColor(Qt::black);
        }
    } else {
        if (listItem) {
            listItem->setTextColor(Qt::gray);
            listItem->setCheckState(Qt::CheckState::Unchecked);       // used for initialization
        }
        if (listDurationItem) {
            listDurationItem->setTextColor(Qt::gray);
        }
    }
}

}
