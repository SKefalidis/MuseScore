//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA
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

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "mscore/musescore.h"
#include "libmscore/album.h"
#include "mscore/preferences.h"

#define DIR QString("libmscore/albums/")

using namespace Ms;

//---------------------------------------------------------
//   TestAlbums
//---------------------------------------------------------

class TestAlbums : public QObject, public MTest
{
    Q_OBJECT

    void saveAlbumTest(const char* file);

private slots:
    void initTestCase();

    void albums1() { saveAlbumTest("smallPianoAlbumTest"); };
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestAlbums::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   saveAlbumTest
//   read a .msca (Album) file, write to a new file and verify both files are identical
//---------------------------------------------------------

void TestAlbums::saveAlbumTest(const char* file)
{
    MScore::debugMode = true;
    Album* album = readAlbum(DIR + QString(file) + ".msca");
    QVERIFY(album);
    QVERIFY(saveAlbum(album, QString(file) + "_auto" + ".msca"));
    QVERIFY(saveCompareAlbum(album, QString(file) + "_auto" + ".msca", DIR + QString(file) + ".msca"));
    delete album;
}

QTEST_MAIN(TestAlbums)
#include "tst_albums.moc"
