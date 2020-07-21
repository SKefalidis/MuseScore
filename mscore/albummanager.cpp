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
#include "scoretab.h"
#include "preferences.h"
#include "icons.h"
#include "libmscore/mscore.h"
#include "libmscore/xml.h"
#include "libmscore/undo.h"
#include "libmscore/album.h"
#include "libmscore/system.h"
#include "libmscore/page.h"
#include "libmscore/box.h"
#include "libmscore/box.h"

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
    setFloating(false);

    // buttons
    up->setIcon(*icons[int(Icons::arrowUp_ICON)]);
    down->setIcon(*icons[int(Icons::arrowDown_ICON)]);
    playButton->setIcon(*icons[int(Icons::play_ICON)]);
    rewindButton->setIcon(*icons[int(Icons::start_ICON)]);

    connect(albumTitleEdit,     &QLineEdit::textChanged,    this, &AlbumManager::albumNameChanged);
    connect(add,                &QPushButton::clicked,      this, &AlbumManager::addClicked);
    connect(addNew,             &QPushButton::clicked,      this, &AlbumManager::addNewClicked);
    connect(up,                 &QPushButton::clicked,      this, &AlbumManager::upClicked);
    connect(down,               &QPushButton::clicked,      this, &AlbumManager::downClicked);
    connect(remove,             &QPushButton::clicked,      this, &AlbumManager::removeClicked);
    connect(deleteButton,       &QPushButton::clicked,      this, &AlbumManager::deleteClicked);
    connect(albumModeButton,    &QRadioButton::toggled,     this, &AlbumManager::changeMode);
    connect(scoreModeButton,    &QRadioButton::toggled,     this, &AlbumManager::changeMode);
    connect(settingsButton,     &QPushButton::clicked,      this, &AlbumManager::openSettingsDialog);
    connect(playButton,         &QToolButton::clicked,      this, static_cast<void (AlbumManager::*)(bool)>(&AlbumManager::playAlbum));
    connect(rewindButton,       &QToolButton::clicked,      this, static_cast<void (AlbumManager::*)(bool)>(&AlbumManager::rewindAlbum));
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
    mscore->restoreGeometry(this);
    m_album = std::unique_ptr<Album>(new Album()); // placeholder

    connect(mscore->getTab1(), &ScoreTab::currentScoreViewChanged, this, &AlbumManager::tabChanged);
    connect(mscore->getTab1(), &ScoreTab::tabRemoved, this, &AlbumManager::tabRemoved);
}

AlbumManager::~AlbumManager()
{
    if (isVisible()) {
        mscore->saveGeometry(this);
        mscore->saveState();
    }
}

//---------------------------------------------------------
//   eventFilter
///     Used to handle drag & drop.
//---------------------------------------------------------

bool AlbumManager::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == scoreList->viewport() && ev->type() == QEvent::DragEnter) {
        QDragEnterEvent* dEvent = static_cast<QDragEnterEvent*>(ev);
        QPoint pos = dEvent->pos();
        m_dragEnterIndex = scoreList->rowAt(pos.y());
    }
    if (obj == scoreList->viewport() && ev->type() == QEvent::Drop) {
        QDropEvent* dEvent = static_cast<QDropEvent*>(ev);
        QPoint pos = dEvent->pos();
        m_dropIndex = scoreList->rowAt(pos.y());
        if (m_dropIndex > m_dragEnterIndex) {
            for (int i = m_dragEnterIndex; i < m_dropIndex; i++) {
                swap(i, i + 1);
            }
        } else if (m_dropIndex < m_dragEnterIndex) {
            for (int i = m_dragEnterIndex; i > m_dropIndex; i--) {
                swap(i, i - 1);
            }
        }
        scoreList->setCurrentCell(m_dropIndex, scoreList->currentColumn());
        ev->ignore();
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void AlbumManager::showEvent(QShowEvent* e)
{
    QDockWidget::showEvent(e);
    activateWindow();
    setFocus();
    getAction("toggle-album")->setChecked(true);
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void AlbumManager::hideEvent(QHideEvent* event)
{
    MuseScore::saveGeometry(this);
    QDockWidget::hideEvent(event);
    getAction("toggle-album")->setChecked(false);
}

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void AlbumManager::retranslate()
{
    retranslateUi(this);
}

//---------------------------------------------------------
//   changeMode
///     Change between score mode and album mode.
//---------------------------------------------------------

void AlbumManager::changeMode(bool checked)
{
    Q_UNUSED(checked);
    disconnect(mscore->getTab1(), &ScoreTab::currentScoreViewChanged, this, &AlbumManager::tabChanged);
    if (scoreModeButton->isChecked()) {
        albumModeButton->setChecked(false);
        if (m_tempScoreTabIndex == mscore->getTab1()->currentIndex()) {
            mscore->openScore(m_album->albumItems().at(0)->fileInfo.absoluteFilePath());
        }
    } else if (albumModeButton->isChecked()) {
        if (!m_album->getDominant()) {
            MasterScore* m_tempScore = m_items.at(0)->albumItem.score->clone(); // clone breaks editing sync for the 1st movement
            while (m_tempScore->systems().size() > 1) { // remove the measures of the cloned masterscore, that way editing is synced
                for (auto x : m_tempScore->systems().last()->measures()) {
                    m_tempScore->removeElement(x);
                }
                m_tempScore->systems().removeLast();
            }

            m_tempScore->setEmptyMovement(true);
            m_tempScore->setfirstRealMovement(1);
            m_tempScore->setName("Temporary Album Score");
            m_tempScore->setPartOfActiveAlbum(true);
            m_album->setDominant(m_tempScore);

            mscore->setCurrentScoreView(mscore->appendScore(m_tempScore));
            mscore->getTab1()->setTabText(mscore->getTab1()->currentIndex(), "Temporary Album Score");
            m_tempScoreTabIndex = mscore->getTab1()->currentIndex();

            for (auto item : m_items) {
                cout << "adding score: " << item->albumItem.score->title().toStdString() << endl;
                m_tempScore->addMovement(item->albumItem.score);
            }
            m_tempScore->setLayoutAll();
            m_tempScore->update();
        } else {
            if (m_tempScoreTabIndex != -1) {
                mscore->setCurrentScoreView(m_tempScoreTabIndex);
            } else {
                mscore->setCurrentScoreView(mscore->appendScore(m_album->getDominant()));
                mscore->getTab1()->setTabText(mscore->getTab1()->currentIndex(), "Temporary Album Score");
                m_tempScoreTabIndex = mscore->getTab1()->currentIndex();
            }
        }
        scoreModeButton->setChecked(false);
    } else {
        Q_ASSERT(false);
    }
    connect(mscore->getTab1(), &ScoreTab::currentScoreViewChanged, this, &AlbumManager::tabChanged);
}

//---------------------------------------------------------
//   tabChanged
//---------------------------------------------------------

void AlbumManager::tabChanged()
{
    albumModeButton->blockSignals(true);
    scoreModeButton->blockSignals(true);
    if (mscore->getTab1()->currentIndex() == m_tempScoreTabIndex && scoreModeButton->isChecked()) {
        albumModeButton->setChecked(true);
    } else if (mscore->getTab1()->currentIndex() != m_tempScoreTabIndex && albumModeButton->isChecked()) {
        scoreModeButton->setChecked(true);
    }
    changeMode(true);
    albumModeButton->blockSignals(false);
    scoreModeButton->blockSignals(false);
}

//---------------------------------------------------------
//   tabRemoved
//---------------------------------------------------------

void AlbumManager::tabRemoved(int index)
{
    albumModeButton->blockSignals(true);
    scoreModeButton->blockSignals(true);
    if (index == m_tempScoreTabIndex) {
        scoreModeButton->setChecked(true);
        m_tempScoreTabIndex = -1;
        changeMode(true);
    } else if (index < m_tempScoreTabIndex) {
        m_tempScoreTabIndex--;
    } else if (index - 1 == m_tempScoreTabIndex) {
        albumModeButton->setChecked(true);
        changeMode(true);
    }
    scoreModeButton->blockSignals(false);
    albumModeButton->blockSignals(false);
}

//---------------------------------------------------------
//   albumNameChanged
//      TODO_SK: make sure that the 4 text fields exist, like in the contents
//---------------------------------------------------------

void AlbumManager::albumNameChanged(const QString& text)
{
    if (!m_album->getDominant()) {
        return;
    }

    VBox* box = toVBox(m_album->getDominant()->measures()->first());
    qreal pageHeight = m_album->getDominant()->pages().at(0)->height();
    qreal scoreSpatium = m_album->getDominant()->spatium();

    box->setOffset(0, pageHeight * 0.1);
    box->setBoxHeight(Spatium(pageHeight * 0.8 / scoreSpatium));

    for (auto x : m_album->getDominant()->measures()->first()->el()) {
        if (x && x->isText()) {
            Text* t = toText(x);

            if (t->tid() == Tid::TITLE) {
                t->setFontStyle(FontStyle::Bold); // I should be calling t->setBold(true) (this overwrites other styles) but it crashes
                t->setSize(36);

                t->cursor()->setRow(0);
                t->setPlainText(text);
            } else if (t->tid() == Tid::SUBTITLE) {
                t->setSize(24);
                t->setAlign(Align::HCENTER | Align::BASELINE);

                t->cursor()->setRow(0);
                t->setPlainText("");
            } else if (t->tid() == Tid::COMPOSER) {
                t->setSize(16);
                t->setPlainText(m_album->composers().join("\n"));
            } else if (t->tid() == Tid::POET) {
                t->setSize(16);
                t->setPlainText(m_album->lyricists().join("\n"));
            }
        }
    }
    updateContents();
}

//---------------------------------------------------------
//   updateContents
//---------------------------------------------------------

void AlbumManager::updateContents()
{
    if (!m_album->getDominant()) {
        return;
    }

    if (!album()->generateContents()) {
        return;
    }

    qreal pageWidth = m_album->getDominant()->pages().at(0)->width();
    qreal scoreSpatium = m_album->getDominant()->spatium();
    int charWidth = pageWidth / scoreSpatium;

    if (!m_album->getDominant()->movements()->at(1)->emptyMovement()) {    // there is no contents page
        MasterScore* ms = m_items.at(0)->albumItem.score->clone();
        ms->setName("Contents");
        ms->setEmptyMovement(true);
        m_album->getDominant()->insertMovement(ms, 1);

        while (ms->systems().size() > 1) {
            for (auto x : ms->systems().last()->measures()) {
                ms->removeElement(x);
            }
            ms->systems().removeLast();
        }

        // make sure that we have these 2 text fields
        MeasureBase* measure = ms->measures()->first();
        measure->clearElements();
        Text* s = new Text(ms, Tid::TITLE);
        s->setPlainText("");
        measure->add(s);
        s = new Text(ms, Tid::SUBTITLE);
        s->setPlainText("");
        measure->add(s);
    }

    MasterScore* ms = m_album->getDominant()->movements()->at(1);
    for (auto x : ms->measures()->first()->el()) {
        if (x && x->isText()) {
            Text* t = toText(x);

            if (t->tid() == Tid::TITLE) {
                t->setFontStyle(FontStyle::Bold); // I should be calling t->setBold(true) (this overwrites other styles) but it crashes
                t->setSize(36);

                t->cursor()->setRow(0);
                t->setPlainText("Contents");
            } else if (t->tid() == Tid::SUBTITLE) {
                t->setSize(16);
                t->setAlign(Align::LEFT | Align::BASELINE);

                QString str("");

                int i = 0;
                for (auto x : m_album->scoreTitles()) {
                    QString temp(x);
                    temp.append(QString(".").repeated(charWidth - x.length()));
                    temp += QString::number(m_album->albumItems().at(i)->score->pageIndexInAlbum());
                    temp += "\n";
                    str += temp;
                    i++;
                }

                t->cursor()->setRow(0);
                t->setPlainText(str);
            } else if (t->tid() == Tid::COMPOSER) {
                t->setSize(16);
                t->setPlainText("");
            } else if (t->tid() == Tid::POET) {
                t->setSize(16);
                t->setPlainText("");
            }
        }
    }
    ms->doLayout();
}

//---------------------------------------------------------
//   playAlbum
///     TODO_SK: rethink this
//---------------------------------------------------------

void AlbumManager::playAlbum()
{
    static qreal pause { 3 };

    // pause playback
    if (!playButton->isChecked() && seq->isPlaying()) {
        disconnect(seq, &Seq::stopped, this, static_cast<void (AlbumManager::*)()>(&AlbumManager::playAlbum));
        seq->stop();
        m_continuing = true;
        return;
    }

    // connection used to move to the next score automatically during playback
    connect(seq, &Seq::stopped, this, static_cast<void (AlbumManager::*)()>(&AlbumManager::playAlbum), Qt::ConnectionType::UniqueConnection);

    if (m_playbackIndex == -1) {
        m_playbackIndex++;
    }

    if (!m_continuing) {
        if (m_playbackIndex < int(m_items.size())) {
            if (m_items.at(m_playbackIndex)->albumItem.enabled()) {
                //
                // setup score to play
                //
                if (scoreModeButton->isChecked()) {
                    if (m_items.at(m_playbackIndex)->albumItem.score) {
                        // TODO_SK: It should not open a new score (it already has one) it should append the score to MuseScoreCore's scoreList
                        mscore->openScore(m_items.at(m_playbackIndex)->albumItem.fileInfo.absoluteFilePath());         // what if the files have not been saved?
                    } else {
                        Q_ASSERT(false);
                        std::cout << "There is some kind of problem... AlbumManager::playAlbum" << std::endl;
                        m_items.at(m_playbackIndex)->albumItem.score = mscore->openScore(m_items.at(m_playbackIndex)->albumItem.fileInfo.absoluteFilePath());
                    }
                    mscore->currentScoreView()->gotoMeasure(m_items.at(m_playbackIndex)->albumItem.score->firstMeasure());       // rewind before playing
                } else {
                    seq->setNextMovement(m_playbackIndex + 1); // first movement is empty
                    mscore->currentScoreView()->gotoMeasure(seq->score()->firstMeasure());       // rewind before playing
                }
                //
                // start playback
                //
                if (m_playbackIndex == 0) {
                    startPlayback();
                    pause = seq->score()->lastMeasure()->pause() * 1000;
                } else {
                    QTimer::singleShot(pause, this, &AlbumManager::startPlayback);
                    pause = seq->score()->lastMeasure()->pause() * 1000;
                }
                m_playbackIndex++;
            } else { // skip this score
                m_playbackIndex++;
                playAlbum();
            }
        } else { // album ended, reset for
            rewindAlbum();
            disconnect(seq, &Seq::stopped, this, static_cast<void (AlbumManager::*)()>(&AlbumManager::playAlbum));
            m_continuing = false;
            playButton->setChecked(false);
            return;
        }
    } else {
        startPlayback();
        m_continuing = false;
    }

    mscore->currentScoreView()->setActiveScore(m_items.at(m_playbackIndex - 1)->albumItem.score);
}

void AlbumManager::playAlbum(bool checked)
{
    Q_UNUSED(checked);

    playAlbum();
}

//---------------------------------------------------------
//   rewindAlbum
//---------------------------------------------------------

void AlbumManager::rewindAlbum(bool checked)
{
    Q_UNUSED(checked);

    m_playbackIndex = 0;
    m_continuing = false;
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
///     Called when the scoreList/View is reordered.\n
///     (e.g. Drag&Drop)
//---------------------------------------------------------

void AlbumManager::updateScoreOrder(QModelIndex sourceParent, int sourceStart, int sourceEnd,
                                    QModelIndex destinationParent, int destinationRow)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceStart);
    Q_UNUSED(sourceEnd);
    Q_UNUSED(destinationParent);
    Q_UNUSED(destinationRow);

    for (int i = 0; i < int(m_items.size()); i++) {
        for (int j = 0; j < int(m_items.size()); j++) {
            if (m_items.at(j)->albumItem.score->title() != scoreList->item(j, 0)->text()) {
                int h = scoreList->row(scoreList->findItems(m_items.at(j)->albumItem.score->title(),
                                                            Qt::MatchExactly).first());
                std::swap(m_items.at(j), m_items.at(h));
                break;
            } else if (j == int(m_items.size()) - 1) {
                goto exit_loops;
            }
        }
    }
    exit_loops:;
    updateButtons();
}

//---------------------------------------------------------
//   openSettingsDialog
///     (Re)Open the settings dialog menu.
//---------------------------------------------------------

void AlbumManager::openSettingsDialog(bool checked)
{
    Q_UNUSED(checked);

    if (!m_settingsDialog) {
        m_settingsDialog = new AlbumManagerDialog(this);
        m_settingsDialog->start();
    } else {
        m_settingsDialog->start();
    }
}


//---------------------------------------------------------
//   albumScores
///     this is a leftover before the refactor
//---------------------------------------------------------

const std::vector<AlbumItem*> AlbumManager::albumScores() const
{
    return m_album->albumItems();
}

//---------------------------------------------------------
//   addClicked
///     Add an existing score to the Album.\n
///     Opens a dialog to select a Score from the filesystem.
///     so this does not work
//---------------------------------------------------------

void AlbumManager::addClicked(bool checked)
{
    Q_UNUSED(checked);

    QStringList files = mscore->getOpenScoreNames(
        tr("MuseScore Files") + " (*.mscz *.mscx);;", tr("Load Score")
        );
    for (const QString& fn : files) {
        MasterScore* score = mscore->readScore(fn);
        m_album->addScore(score);
        addAlbumItem(*m_album->albumItems().back()); // TODO_SK: Convert to reference and use the unique ptr reference
    }
}

//---------------------------------------------------------
//   addNewClicked
///     Add a new Score to the Album.
//---------------------------------------------------------

void AlbumManager::addNewClicked(bool checked)
{
    Q_UNUSED(checked);

    MasterScore* score = mscore->getNewFile();
    if (!score) {
        return;
    }
    m_album->addScore(score);
    addAlbumItem(*m_album->albumItems().back()); // TODO_SK: Convert to reference and use the unique ptr reference
}

//---------------------------------------------------------
//   addAlbumItem
///     add the given AlbumItem to the AlbumManager
///     creates the corresponding scoreList/View item
///     the AlbumItem and Widget are saved in a new AlbumManagerItem
//---------------------------------------------------------

void AlbumManager::addAlbumItem(AlbumItem& albumItem)
{
    scoreList->blockSignals(true);
    QString name = albumItem.score->title();
    QTableWidgetItem* li = new QTableWidgetItem(name);
    scoreList->setRowCount(scoreList->rowCount() + 1);
    scoreList->setItem(scoreList->rowCount() - 1, 0, li);
    li->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable));
    li->setCheckState(Qt::CheckState::Checked);
    QTableWidgetItem* tid = new QTableWidgetItem("00:00:00");
    scoreList->setItem(scoreList->rowCount() - 1, 1, tid);
    tid->setFlags(Qt::ItemFlags(Qt::ItemIsEnabled));
    AlbumManagerItem* albumManagerItem = new AlbumManagerItem(albumItem, li, tid);
    m_items.push_back(albumManagerItem);
    scoreList->blockSignals(false);
    connect(albumManagerItem, &AlbumManagerItem::durationChanged, this, &AlbumManager::updateTotalDuration);
    albumManagerItem->updateDurationLabel();

    // update the combined score to reflect the changes
    if (m_album->getDominant()) {
        m_album->getDominant()->addMovement(albumItem.score);
        m_album->getDominant()->update();
        m_album->getDominant()->doLayout(); // position the movements correctly
        mscore->currentScoreView()->update(); // repaint
    }
}

//---------------------------------------------------------
//   updateDurations
///     Calculates and updates (the labels of) the duration
///     of the Album and of each individual score.
//---------------------------------------------------------

void AlbumManager::updateDurations()
{
    scoreList->blockSignals(true);
    for (auto item : m_items) {
        item->updateDurationLabel();
    }
    updateTotalDuration();
    scoreList->blockSignals(false);
}

//---------------------------------------------------------
//   updateTotalDuration
//---------------------------------------------------------

void AlbumManager::updateTotalDuration()
{
    scoreList->blockSignals(true);
    int seconds = 0; // total duration
    for (auto item : m_items) {
        if (item->albumItem.enabled()) {
            seconds += item->albumItem.duration();
        }
    }
    durationLabel->setText(durationToString(seconds));
    scoreList->blockSignals(false);
}

//---------------------------------------------------------
//   durationToString
//---------------------------------------------------------

QString durationToString(int seconds)
{
    int tempSeconds = seconds;
    int tempMinutes = tempSeconds / 60;
    tempSeconds -= tempMinutes * 60;
    int tempHours = tempMinutes / 60;
    tempMinutes -= tempHours * 60;

    return (QString::number(tempHours).rightJustified(2, '0') + ":"
        + QString::number(tempMinutes).rightJustified(2, '0') + ":"
        + QString::number(tempSeconds).rightJustified(2, '0'));
}

//---------------------------------------------------------
//   upClicked
///     Up arrow clicked.
//---------------------------------------------------------

void AlbumManager::upClicked(bool checked)
{
    Q_UNUSED(checked);

    int index = scoreList->currentRow();
    if (index == -1 || index == 0) {
        return;
    }
    swap(index, index - 1);
    scoreList->setCurrentCell(index - 1, 0);
}

//---------------------------------------------------------
//   downClicked
///     Down arrow clicked.
//---------------------------------------------------------

void AlbumManager::downClicked(bool checked)
{
    Q_UNUSED(checked);

    int index = scoreList->currentRow();
    if (index == -1 || index == scoreList->rowCount() - 1) {
        return;
    }
    swap(index, index + 1);
    scoreList->setCurrentCell(index + 1, 0);
}

//---------------------------------------------------------
//   itemDoubleClicked
///     Called when one of the Widgets in the scoreList/View
///     gets clicked. \n
///     In Score mode:
///     This either opens the clicked Score or changes to the
///     corresponding tab if it is already open. \n
///     This centers the view to the part of the tempScore
///     where the clicked Score begins.
//---------------------------------------------------------

void AlbumManager::itemDoubleClicked(QTableWidgetItem* item)
{
    AlbumManagerItem* aItem { nullptr };
    for (auto x : m_items) {
        if (x->listItem == item) {
            aItem = x;
        }
    }

    if (!aItem) {
        qDebug("Could not find the clicked AlbumManagerItem.");
        return;
    }

    if (scoreModeButton->isChecked()) {
        mscore->openScore(aItem->albumItem.fileInfo.absoluteFilePath());
        aItem->albumItem.score->doLayout();
    } else {
        mscore->currentScoreView()->gotoMeasure(aItem->albumItem.score->firstMeasure()); // move to the chosen measure
        mscore->currentScoreView()->deselectAll(); // deselect the element selected by `goToMeasure`
        mscore->currentScoreView()->updateAll();
    }
}

//---------------------------------------------------------
//   swap
///     Swap the 2 given AlbumScores.
//---------------------------------------------------------

void AlbumManager::swap(int indexA, int indexB)
{
    //
    // The problem is that the widgets are contained in both the AlbumScores and their
    // main container, that's why I need to swap them multiple times.
    //
    scoreList->blockSignals(true);
    m_album->swap(indexA, indexB);
    // swap them in their container
    std::swap(m_items.at(indexA), m_items.at(indexB));

    // swap the text of the widgets
    QTableWidgetItem* itemA = scoreList->item(indexA, 0);
    itemA->setText(m_items.at(indexA)->albumItem.score->title());
    QTableWidgetItem* itemB = scoreList->item(indexB, 0);
    itemB->setText(m_items.at(indexB)->albumItem.score->title());

    // swap again the widgets to place them correctly FIXME: isn't there a better way to do all this?
    std::swap(m_items.at(indexA)->listItem, m_items.at(indexB)->listItem);   // workaround, because the list widget items are changed twice so they are being reset
    std::swap(m_items.at(indexA)->listDurationItem, m_items.at(indexB)->listDurationItem);

    // update the enabled indicators
    m_items.at(indexA)->setEnabled(m_items.at(indexA)->albumItem.enabled());
    m_items.at(indexB)->setEnabled(m_items.at(indexB)->albumItem.enabled());

    // update the duration labels
    updateDurations();
    scoreList->blockSignals(false);

    // update the combined score to reflect the changes
    if (m_album->getDominant()) {
        std::swap(m_album->getDominant()->movements()->at(indexA + 1), m_album->getDominant()->movements()->at(indexB + 1));
        // these should probably only run if the current tab is the one with the tempScore
//        tempScore->update(); probably not needed
        m_album->getDominant()->doLayout(); // position the movements correctly
        mscore->currentScoreView()->update(); // repaint
    }
}

//---------------------------------------------------------
//   removeClicked
//---------------------------------------------------------

void AlbumManager::removeClicked(bool checked)
{
    Q_UNUSED(checked);

    m_items.erase(m_items.begin() + scoreList->currentRow());
    m_album->removeScore(scoreList->currentRow());
    scoreList->removeRow(scoreList->currentRow());
    updateDurations();
}

//---------------------------------------------------------
//   deleteClicked
//---------------------------------------------------------

void AlbumManager::deleteClicked(bool checked)
{
    Q_UNUSED(checked);
}

//---------------------------------------------------------
//   setAlbum
//---------------------------------------------------------

void AlbumManager::setAlbum(std::unique_ptr<Album> a)
{
    std::cout << "setting album" << std::endl;
    scoreList->setRowCount(0);
    m_items.clear(); // TODO_SK: also free all
    for (auto x : m_album->albumItems()) {
        m_album->removeScore(x->score);
    }
    Album::activeAlbum = nullptr;
    if (!a)
        return;

    m_album = std::move(a);

    scoreList->blockSignals(true);
    for (auto& item : m_album->albumItems()) {
        QString path = item->fileInfo.canonicalFilePath();
        MasterScore* score = mscore->openScoreWithoutAppending(path);
        score->updateCapo();
        item->setScore(score);
        addAlbumItem(*item); // TODO_SK: Convert to reference and use the unique ptr reference
    }

    m_album->addSectionBreaks(); // TODO_SK: normally I should add the sections breaks while loading, but the scores haven't been loaded so I can't
    scoreList->blockSignals(false);

    Album::activeAlbum = m_album.get();
}

//---------------------------------------------------------
//   album
//---------------------------------------------------------

const std::unique_ptr<Album>& AlbumManager::album() const
{
    return m_album;
}

//---------------------------------------------------------
//   updateButtons
///     Activates/Deactivates buttons depending on the selected row
///     and whether there are Scores in the Album.
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
///     Called when the state of the item changes.
///     Updates the duration and whether is it enabled.
//---------------------------------------------------------

void AlbumManager::itemChanged(QTableWidgetItem* item)
{
    scoreList->blockSignals(true);
    if (item->column() == 0) {
        AlbumManagerItem* albumManagerItem = m_items.at(scoreList->row(item));
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

void MuseScore::showAlbumManager(bool visible)
{
    QAction* toggleAlbumManagerAction = getAction("toggle-album");

    if (albumManager == 0) {
        albumManager = new AlbumManager(this);
    }

    reDisplayDockWidget(albumManager, visible);

    if (visible) {
        albumManager->show();
        albumManager->albumTitleEdit->setFocus();
    }

    toggleAlbumManagerAction->setChecked(visible);
}

//---------------------------------------------------------
//---------------------------------------------------------
//
//   AlbumManagerItem
//
//---------------------------------------------------------
//---------------------------------------------------------

AlbumManagerItem::AlbumManagerItem(AlbumItem& item, QTableWidgetItem* listItem, QTableWidgetItem* listDurationItem)
    : albumItem(item)
{
    if (!albumItem.score) {
        QString path = item.fileInfo.canonicalFilePath();
        MasterScore* score = mscore->readScore(path);
        albumItem.setScore(score);
    }
    albumItem.score->setPartOfActiveAlbum(true);
    this->listItem = listItem;
    this->listDurationItem = listDurationItem;
    setEnabled(albumItem.enabled());
    connect(&albumItem, &AlbumItem::durationChanged, this, &AlbumManagerItem::updateDurationLabel);
}

AlbumManagerItem::~AlbumManagerItem()
{
    albumItem.score->setPartOfActiveAlbum(false);
}

//---------------------------------------------------------
//   setEnabled
//---------------------------------------------------------

void AlbumManagerItem::setEnabled(bool b)
{
    albumItem.setEnabled(b);
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

//---------------------------------------------------------
//   updateDurationLabel
//---------------------------------------------------------

void AlbumManagerItem::updateDurationLabel()
{
    int tempSeconds = albumItem.score->duration();
    listDurationItem->setText(durationToString(tempSeconds));
    emit durationChanged();
}

}
