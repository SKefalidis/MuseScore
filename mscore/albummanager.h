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

#ifndef __ALBUMMANAGER_H__
#define __ALBUMMANAGER_H__

#include "ui_albummanager.h"
#include "abstractdialog.h"

namespace Ms {

class XmlWriter;
class Movements;
class MasterScore;
class Score;
class AlbumManagerDialog;

//---------------------------------------------------------
//   AlbumScore
//---------------------------------------------------------

struct AlbumScore : public QObject {
      Q_OBJECT

   public:
      MasterScore* score                  { nullptr };
      QTableWidgetItem* listItem          { nullptr };
      QTableWidgetItem* listDurationItem  { nullptr };
      bool enabled                        { true };
      QString filePath                    { "-" };

      AlbumScore(MasterScore* score, QTableWidgetItem* listItem, QTableWidgetItem* listDurationItem, bool enabled = true);
      ~AlbumScore();
      void setEnabled(bool b);
      void setScore(MasterScore* score);
      };

//---------------------------------------------------------
//   Album
//---------------------------------------------------------

struct Album : public QObject {
      Q_OBJECT

   public:
      std::vector<AlbumScore*> _albumScores     {};
      std::string _albumTitle                   { "" };
      Album();
      };

//---------------------------------------------------------
//   AlbumManager
//---------------------------------------------------------

class AlbumManager final : public QDockWidget, public Ui::AlbumManager {
      Q_OBJECT

   private:
      AlbumManagerDialog* settingsDialog        { nullptr };
      std::vector<AlbumScore*> _albumScores     {};
      MasterScore* tempScore                    { nullptr };

      int dragEnterIndex                        { -1 };
      int dropIndex                             { -1 };

      virtual void hideEvent(QHideEvent*) override;
      void updateDurations();

   private slots:
      void addClicked(bool throwaway = false);
      void addNewClicked(bool throwaway = false);
      void upClicked(bool throwaway = false);
      void downClicked(bool throwaway = false);
      void itemDoubleClicked(QTableWidgetItem* item);
      void swap(int indexA, int indexB);
      void removeClicked(bool throwaway = false);
      void deleteClicked(bool throwaway = false);
      void updateButtons();
      void itemChanged(QTableWidgetItem* item);   // score name in list is edited
      void changeMode(bool throwaway = false);
      void playAlbum();
      void playAlbum(bool throwaway);
      void startPlayback();
      void updateScoreOrder(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent, int destinationRow);
      void openSettingsDialog(bool throwaway = false);

   protected:
      virtual void retranslate() { retranslateUi(this); }
      bool eventFilter(QObject *obj, QEvent *ev) override;

   public:
      AlbumManager(QWidget* parent = 0);
      ~AlbumManager();
      void setAlbum(Movements*);
      void addScore(MasterScore* score, bool enabled = true);
      void saveCurrentAlbum(XmlWriter& xml);

      std::vector<AlbumScore*> albumScores() const { return _albumScores; }

      int playbackDelay {1000};
      };
}

#endif

