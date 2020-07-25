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

#define DIR QString("libmscore/albumsIO/")

using namespace Ms;

//---------------------------------------------------------
//   TestAlbums
//---------------------------------------------------------

class TestAlbumsIO : public QObject, public MTest
{
    Q_OBJECT

    void saveAlbumTest(const char* file);
    void stringsTest(const char* file);

private slots:
    void initTestCase();

    void albumsIO() { saveAlbumTest("smallPianoAlbumTest"); };
    void albumsStrings() { stringsTest("smallPianoAlbumTest"); };
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestAlbumsIO::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   saveAlbumTest
//   read a .msca (Album) file, write to a new file and verify both files are identical
//---------------------------------------------------------

void TestAlbumsIO::saveAlbumTest(const char* file)
{
    MScore::debugMode = true;
    Album* album = readAlbum(DIR + QString(file) + ".msca");
    QVERIFY(album);
    QFileInfo fi(QString(file) + "_auto" + ".msca");
    QVERIFY(saveAlbum(album, fi.absoluteFilePath()));   // wrong path, but not deleted for debugging
    QVERIFY(saveCompareAlbum(album, DIR + QString(file) + "_auto" + ".msca", DIR + QString(file) + ".msca"));
    delete album;
}

//---------------------------------------------------------
//   stringsTest
//---------------------------------------------------------

void TestAlbumsIO::stringsTest(const char* file)
{
    MScore::debugMode = true;
    Album* album = readAlbum(DIR + QString(file) + ".msca");
    QVERIFY(album);
    auto x = album->composers();
    QVERIFY(x.size() == 2);
    QCOMPARE(x.at(0), "Sergios - Anestis Kefalidis");
    QCOMPARE(x.at(1), "Oregano");
    auto y = album->composers();
    QVERIFY(y.size() == 1);
    QCOMPARE(y.at(0), "Garlic");
    auto z = album->scoreTitles();
    QVERIFY(z.size() == 3);
    QCOMPARE(z.at(0), "Piano 1");
    QCOMPARE(z.at(1), "Piano 2");
    QCOMPARE(z.at(2), "Piano 3");
    delete album;
}

QTEST_MAIN(TestAlbumsIO)
#include "tst_albumsIO.moc"
