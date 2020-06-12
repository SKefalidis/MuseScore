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
class Album;
struct AlbumItem;

//---------------------------------------------------------
//   AlbumManagerItem
//---------------------------------------------------------

struct AlbumManagerItem : public QObject {
    Q_OBJECT

public:
    // can I convert these to references?
    AlbumItem* albumItem;
    QTableWidgetItem* listItem;
    QTableWidgetItem* listDurationItem;

    AlbumManagerItem(AlbumItem* albumItem, QTableWidgetItem* listItem, QTableWidgetItem* listDurationItem);
    ~AlbumManagerItem();
    void setEnabled(bool b);
};

//---------------------------------------------------------
//   AlbumManager
//---------------------------------------------------------

class AlbumManager final : public QDockWidget, public Ui::AlbumManager
{
    Q_OBJECT

private:
    AlbumManagerDialog* settingsDialog        { nullptr };
    Album* _album                             { nullptr };
    std::vector<AlbumManagerItem*> _items     {};
    MasterScore* tempScore                    { nullptr };

    int dragEnterIndex                        { -1 };
    int dropIndex                             { -1 };

    virtual void hideEvent(QHideEvent*) override;
    void updateDurations();

private slots:
    void addClicked(bool throwaway = false);
    void addNewClicked(bool throwaway = false);
    void addAlbumItem(AlbumItem* albumItem);
    void upClicked(bool throwaway = false);
    void downClicked(bool throwaway = false);
    void itemDoubleClicked(QTableWidgetItem* item);
    void swap(int indexA, int indexB);
    void removeClicked(bool throwaway = false);
    void deleteClicked(bool throwaway = false);
    void updateButtons();
    void itemChanged(QTableWidgetItem* item);     // score name in list is edited
    void changeMode(bool throwaway = false);
    void playAlbum(); // TODO: move to libmscore
    void playAlbum(bool throwaway);
    void startPlayback();
    void updateScoreOrder(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent,
                          int destinationRow);
    void openSettingsDialog(bool throwaway = false);

protected:
    virtual void retranslate() { retranslateUi(this); }
    bool eventFilter(QObject* obj, QEvent* ev) override;

public:
    AlbumManager(QWidget* parent = 0);
    ~AlbumManager();
    Album* album() { return _album; }
    void setAlbum(Album* album);

    std::vector<AlbumItem*> albumScores() const;
};
}

#endif
