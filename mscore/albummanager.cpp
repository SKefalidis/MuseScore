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
      setWindowFlags(Qt::Tool); // copy paste from play panel
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea)); // copy paste from play panel
      mscore->addDockWidget(Qt::RightDockWidgetArea, this);

      // buttons
      up->setIcon(*icons[int(Icons::arrowUp_ICON)]);
      down->setIcon(*icons[int(Icons::arrowDown_ICON)]);

      connect(add,                  &QPushButton::clicked,  this,       &AlbumManager::addClicked);
      connect(addNew,               &QPushButton::clicked,  this,       &AlbumManager::addNewClicked);
      connect(up,                   &QPushButton::clicked,  this,       &AlbumManager::upClicked);
      connect(down,                 &QPushButton::clicked,  this,       &AlbumManager::downClicked);
      connect(remove,               &QPushButton::clicked,  this,       &AlbumManager::removeClicked);
      connect(deleteButton,         &QPushButton::clicked,  this,       &AlbumManager::deleteClicked);
      connect(albumModeButton,      &QRadioButton::toggled, this,       &AlbumManager::changeMode);
      connect(scoreModeButton,      &QRadioButton::toggled, this,       &AlbumManager::changeMode);
      connect(settingsButton,       &QPushButton::clicked,  this,       &AlbumManager::openSettingsDialog);
      connect(playButton,           &QPushButton::clicked,  this,       static_cast<void (AlbumManager::*)(bool)>(&AlbumManager::playAlbum));
      connect(scoreList,            &QTableWidget::itemChanged, this,               &AlbumManager::itemChanged);
      connect(scoreList,            &QTableWidget::itemDoubleClicked, this,         &AlbumManager::itemDoubleClicked);
      connect(scoreList,            &QTableWidget::itemSelectionChanged, this,      &AlbumManager::updateButtons);
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
      if (isVisible())
            MuseScore::saveGeometry(this);
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
            if (dropIndex > dragEnterIndex)
                  for (int i = dragEnterIndex; i < dropIndex; i++)
                        swap(i, i + 1);
            else if (dropIndex < dragEnterIndex)
                  for (int i = dragEnterIndex; i > dropIndex; i--)
                        swap(i, i - 1);
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
            }
      else if (albumModeButton->isChecked()) {
            scoreModeButton->setChecked(false);
            if (!tempScore) {
                  tempScore = _albumScores.at(0)->score->clone();
                  mscore->setCurrentScoreView(mscore->appendScore(tempScore));
                  mscore->getTab1()->setTabText(mscore->getTab1()->count() - 1, "Temporary Album Score");
                  for (auto albumScore : _albumScores) {
                        if (albumScore == _albumScores.at(0))
                              continue;
                        cout << "adding score: " << albumScore->score->title().toStdString() << endl;
                        tempScore->addMovement(albumScore->score/*->clone()*/); // με το clone διορθώνεται το πρόγραμμα που χαλάνε οι άλλες παρτιτούρες, αλλά δεν έχω άμεση αλλαγή
                        }
                  tempScore->setLayoutAll();
                  tempScore->update();

//                  addScore(tempScore);
                  }
            }
      else {
            Q_ASSERT(false);
            }
      }

//---------------------------------------------------------
//   playAlbum
//---------------------------------------------------------

void AlbumManager::playAlbum()
      {
      static int i {-1};
      i++;

      if (i < _albumScores.size()) {
            if (_albumScores.at(i)->enabled) {
                  if (_albumScores.at(i)->score)
                        mscore->openScore(_albumScores.at(i)->filePath); // what if the files have not been saved?
                  else
                        _albumScores.at(i)->score = mscore->openScore(_albumScores.at(i)->filePath);
                  mscore->currentScoreView()->gotoMeasure(_albumScores.at(i)->score->firstMeasure()); // rewind before playing
                  if (i == 0)
                        seq->start();
                  else
                        QTimer::singleShot(playbackDelay, this, &AlbumManager::startPlayback);
                  if (i == 0)
                        connect(seq, &Seq::stopped, this, static_cast<void (AlbumManager::*)()>(&AlbumManager::playAlbum));
                  }
            else {
                  playAlbum();
                  }
            }
      else {
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

void AlbumManager::updateScoreOrder(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent, int destinationRow)
      {
      for (int i = 0; i < _albumScores.size(); i++) {
            for (int j = 0; j < _albumScores.size(); j++) {
                  if (_albumScores.at(j)->score->title() != scoreList->item(j, 0)->text()) {
                        int h = scoreList->row(scoreList->findItems(_albumScores.at(j)->score->title(), Qt::MatchExactly).first());
                        std::swap(_albumScores.at(j), _albumScores.at(h));
                        break;
                        }
                  else if (j == _albumScores.size() - 1){
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
            }
      else {
            settingsDialog->start();
            }
      }

//---------------------------------------------------------
//   saveCurrentAlbum
//---------------------------------------------------------

void AlbumManager::saveCurrentAlbum(XmlWriter& xml)
      {
      for (int i = 0 ; i < _albumScores.size(); i++) {
            Score* score = _albumScores.at(i)->score;
            xml.stag("Score");
            xml.tag("alias", scoreList->item(i, 0)->text());
            xml.tag("path", score->importedFilePath());
//            xml.tag("relativePath", )
            xml.tag("enabled", _albumScores.at(i)->enabled);
            xml.etag();
            }
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
            if (t)
                  name = QTextDocumentFragment::fromHtml(t->xmlText()).toPlainText().replace("&amp;","&").replace("&gt;",">").replace("&lt;","<").replace("&quot;", "\"");
            name = name.simplified();
            }
      if (name.isEmpty())
            name = score->title();
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
      if (scores.empty())
            return;
      for (MasterScore* score : scores)
            addScore(score);
      }

//---------------------------------------------------------
//   addNewClicked
///   Add a new Score to the Album.
//---------------------------------------------------------

void AlbumManager::addNewClicked(bool throwaway)
      {
      MasterScore* score = mscore->getNewFile();
      if (!score)
            return;
      addScore(score);
      }

//---------------------------------------------------------
//   addScore
///   add the given Score to the Album
///   creates the corresponding scoreList/View item
///   the Score and Widget are saved ina new AlbumScore
//---------------------------------------------------------

void AlbumManager::addScore(MasterScore* score, bool enabled)
      {
      if (!score) {
            cout << "There is no score to add to album..." << endl;
            return;
            }
      cout << "Adding score to album..." << endl;

      scoreList->blockSignals(true);
      QString name = getScoreTitle(score);
      QTableWidgetItem* li = new QTableWidgetItem(name);
      scoreList->setRowCount(scoreList->rowCount() + 1);
      scoreList->setItem(scoreList->rowCount() - 1, 0, li);
      li->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable); // drag and drop is not working
      li->setCheckState(Qt::CheckState::Checked);
      QTableWidgetItem* tid = new QTableWidgetItem("00:00:00");
      scoreList->setItem(scoreList->rowCount() - 1, 1, tid);
      tid->setFlags(Qt::ItemIsEnabled);

      AlbumScore* albumScore = new AlbumScore(score, li, tid, enabled);
      _albumScores.push_back(albumScore);
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

      for (auto albumScore : _albumScores) {
            bool temporarilyOpen = false;
            if (albumScore->score == nullptr) {
                  albumScore->setScore(mscore->openScore(albumScore->filePath, false));
                  temporarilyOpen = true;
                  }

            if (albumScore->enabled)
                  seconds += albumScore->score->duration();

            int tempSeconds = albumScore->score->duration();
            int tempMinutes = tempSeconds / 60;
            tempSeconds -= tempMinutes * 60;
            int tempHours = tempMinutes / 60;
            tempMinutes -= tempHours * 60;

            albumScore->listDurationItem->setText(
                              QString::number(tempHours).rightJustified(2, '0') + ":" +
                              QString::number(tempMinutes).rightJustified(2, '0') + ":" +
                              QString::number(tempSeconds).rightJustified(2, '0'));

            if (temporarilyOpen)
                  mscore->closeScore(albumScore->score);
            }
      minutes = seconds / 60;
      seconds -= minutes * 60;
      hours = minutes / 60;
      minutes -= hours * 60;

      durationLabel->setText(QString::number(hours).rightJustified(2, '0') + ":" + QString::number(minutes).rightJustified(2, '0') + ":" + QString::number(seconds).rightJustified(2, '0'));
      scoreList->blockSignals(false);
      }

//---------------------------------------------------------
//   upClicked
///   Up arrow clicked.
//---------------------------------------------------------

void AlbumManager::upClicked(bool throwaway)
      {
      int index = scoreList->currentRow();
      if (index == -1 || index == 0)
            return;
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
      if (index == -1 || index == scoreList->rowCount() - 1)
            return;
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
            for (auto& x : _albumScores) {
                  if (x->listItem == item) {
                        if (x->score) {
                              mscore->openScore(x->filePath);
                              }
                        else {
                              x->score = mscore->openScore(x->filePath);
                              x->score->setPartOfActiveAlbum(true);
                              }
                        x->score->doLayout();
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
      // swap them in their container
      std::swap(_albumScores.at(indexA), _albumScores.at(indexB));

      // swap the text of the widgets
      QTableWidgetItem* itemA = scoreList->item(indexA, 0);
      itemA->setText(_albumScores.at(indexA)->score->title());
      QTableWidgetItem* itemB = scoreList->item(indexB, 0);
      itemB->setText(_albumScores.at(indexB)->score->title());

      // swap again the widgets to place them correctly FIXME: isn't there a better way to do all this?
      std::swap(_albumScores.at(indexA)->listItem, _albumScores.at(indexB)->listItem); // workaround, because the list widget items are changed twice so they are being reset
      std::swap(_albumScores.at(indexA)->listDurationItem, _albumScores.at(indexB)->listDurationItem);

      // update the enabled indicators
      _albumScores.at(indexA)->setEnabled(_albumScores.at(indexA)->enabled);
      _albumScores.at(indexB)->setEnabled(_albumScores.at(indexB)->enabled);

      // update the duration labels
      updateDurations();
      scoreList->blockSignals(false);
      }

//---------------------------------------------------------
//   removeClicked
//---------------------------------------------------------

void AlbumManager::removeClicked(bool throwaway)
      {
      delete(albumScores().at(scoreList->currentRow()));
      _albumScores.erase(_albumScores.begin() + scoreList->currentRow());
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

void AlbumManager::setAlbum(Movements* a)
      {
//      scoreList->clear();
//      if (!a)
//            return;

//      album = a;

//      scoreList->blockSignals(true);
//      for (MasterScore* score : *album) {
//            QString name = getScoreTitle(score);
//            QTableWidgetItem* li = new QTableWidgetItem(name);
//            scoreList->setRowCount(scoreList->rowCount() + 1);
//            scoreList->setItem(scoreList->rowCount() - 1, 0, li);
//            li->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled));
//            }
//      scoreList->blockSignals(false);
//      add->setEnabled(true);
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
      down->setEnabled(idx < (n-1));
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
            AlbumScore* albumScore = _albumScores.at(scoreList->row(item));
            if (item->checkState() == Qt::CheckState::Checked) {
                  albumScore->setEnabled(true);
                  }
            else {
                  albumScore->setEnabled(false);
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
      if (albumManager == 0)
            albumManager = new AlbumManager(this);

//      if (currentScoreView() && currentScoreView()->score() && currentScoreView()->score()->masterScore()) // add the current score to the album
//            albumManager->addScore(currentScoreView()->score()->masterScore());
      albumManager->show();
      albumManager->albumTitleEdit->setFocus();
      }

//---------------------------------------------------------
//---------------------------------------------------------
//
//   AlbumScore
//
//---------------------------------------------------------
//---------------------------------------------------------

AlbumScore::AlbumScore(MasterScore* score, QTableWidgetItem* listItem, QTableWidgetItem* listDurationItem, bool enabled)
      {
      this->score = score;
      this->score->setPartOfActiveAlbum(true);
      this->listItem = listItem;
      this->listDurationItem = listDurationItem;
      this->filePath = score->importedFilePath();
      setEnabled(enabled);
      }

AlbumScore::~AlbumScore()
      {
      this->score->setPartOfActiveAlbum(false);
      }

//---------------------------------------------------------
//   setEnabled
//---------------------------------------------------------

void AlbumScore::setEnabled(bool b)
      {
      enabled = b;
      if (b) {
            if (listItem) {
                  listItem->setTextColor(Qt::black);
                  listItem->setCheckState(Qt::CheckState::Checked); // used for initialization
                  }
            if (listDurationItem)
                  listDurationItem->setTextColor(Qt::black);
            }
      else {
            if (listItem) {
                  listItem->setTextColor(Qt::gray);
                  listItem->setCheckState(Qt::CheckState::Unchecked); // used for initialization
                  }
            if (listDurationItem)
                  listDurationItem->setTextColor(Qt::gray);
            }
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void AlbumScore::setScore(MasterScore* score)
      {
      if (this->score)
            this->score->setPartOfActiveAlbum(false);
      this->score = score;
      this->score->setPartOfActiveAlbum(true);
      }
}
