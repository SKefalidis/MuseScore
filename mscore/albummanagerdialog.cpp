//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "albummanagerdialog.h"
#include "albummanager.h"
#include "libmscore/album.h"
#include "ui_albummanagerdialog.h"

namespace Ms {

//---------------------------------------------------------
//   AlbumManagerDialog
//---------------------------------------------------------

AlbumManagerDialog::AlbumManagerDialog(QWidget* parent) :
    QDialog(parent)
{
    setObjectName("AlbumManagerDialog");
    setupUi(this);
    connect(buttonBox, &QDialogButtonBox::clicked, this, &AlbumManagerDialog::buttonBoxClicked);
}

AlbumManagerDialog::~AlbumManagerDialog()
{
    delete ui;
}

//---------------------------------------------------------
//   start
//---------------------------------------------------------

void AlbumManagerDialog::start()
{
    update();
    show();
}

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void AlbumManagerDialog::apply()
{
    AlbumManager* albumManager = static_cast<AlbumManager*>(parent());
    albumManager->album()->setDefaultPlaybackDelay(playbackDelayBox->value());
    albumManager->album()->setGenerateContents(checkContentsGeneration->isChecked());
    albumManager->album()->setAddPageBreaksEnabled(checkAddPageBreak->isChecked());
    if (checkAddPageBreak->isChecked()) {
        albumManager->album()->addPageBreaks();
    }
}

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void AlbumManagerDialog::update()
{
    AlbumManager* albumManager = static_cast<AlbumManager*>(parent());
    playbackDelayBox->setValue(albumManager->album()->defaultPlaybackDelay());
    checkContentsGeneration->setChecked(albumManager->album()->generateContents());
    checkAddPageBreak->setChecked(albumManager->album()->addPageBreaksEnabled());
}

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void AlbumManagerDialog::buttonBoxClicked(QAbstractButton* button)
{
    switch (buttonBox->standardButton(button)) {
    case QDialogButtonBox::Apply:
        apply();
        break;
    case QDialogButtonBox::Ok:
        apply();
    // fall through
    case QDialogButtonBox::Cancel:
    default:
        hide();
        break;
    }
}
}
