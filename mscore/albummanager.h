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
    AlbumManagerItem(AlbumItem* albumItem, QTableWidgetItem* listItem, QTableWidgetItem* listDurationItem);
    ~AlbumManagerItem();

    void setEnabled(bool b);

    // can I convert these to references?
    AlbumItem* albumItem;
    QTableWidgetItem* listItem;
    QTableWidgetItem* listDurationItem;
};

//---------------------------------------------------------
//   AlbumManager
//---------------------------------------------------------

class AlbumManager final : public QDockWidget, public Ui::AlbumManager
{
    Q_OBJECT

public:
    AlbumManager(QWidget* parent = 0);
    ~AlbumManager();

    Album* album();
    void setAlbum(Album* album);

    std::vector<AlbumItem*> albumScores() const;

protected:
    virtual void retranslate();
    bool eventFilter(QObject* obj, QEvent* ev) override;

private slots:
    void addAlbumItem(AlbumItem* albumItem);
    void itemDoubleClicked(QTableWidgetItem* item);
    void swap(int indexA, int indexB);
    void updateButtons();
    void itemChanged(QTableWidgetItem* item);     // score name in list is edited

    // The unused 'checked' parameters exist because Qt 5 style signals/slots don't
    // accept default values.
    void addClicked(bool checked = false);
    void addNewClicked(bool checked = false);
    void upClicked(bool checked = false);
    void downClicked(bool checked = false);
    void removeClicked(bool checked = false);
    void deleteClicked(bool checked = false);
    void changeMode(bool checked = false);
    void openSettingsDialog(bool checked = false);
    void playAlbum(bool checked);
    void playAlbum(); // TODO: move to libmscore
    void startPlayback();
    void rewindAlbum(bool checked = false);
    void updateScoreOrder(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent,
                          int destinationRow);

private:
    virtual void showEvent(QShowEvent*) override;
    virtual void hideEvent(QHideEvent*) override;
    void updateDurations();

    AlbumManagerDialog* m_settingsDialog    { nullptr };
    Album* m_album                          { nullptr };
    std::vector<AlbumManagerItem*> m_items  {};
    MasterScore* m_tempScore                { nullptr };

    int m_dragEnterIndex    { -1 };
    int m_dropIndex         { -1 };
    int m_playbackIndex     { -1 };
    bool m_continuing       { false };
};
}

#endif
