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
    void addRemoveTest(const char* file);

private slots:
    void initTestCase();

    void albumsIO() { saveAlbumTest("smallPianoAlbumTest"); }
    void albumsStrings() { stringsTest("smallPianoAlbumTest"); }
    void albumsAddRemove() { addRemoveTest("smallPianoAlbumTest"); }
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

    // load scores
    for (auto& item : album->albumItems()) {
        QString path = item->fileInfo.canonicalFilePath();
        MasterScore* score = readScoreAlbums(path);
        item->setScore(score);
    }

    std::cout << "Loading completed..." << std::endl;

    auto x = album->composers();
    QVERIFY(x.size() == 2);
    QCOMPARE(x.at(0), QString("Sergios - Anestis Kefalidis"));
    QCOMPARE(x.at(1), QString("Oregano"));
    auto y = album->lyricists();
    QVERIFY(y.size() == 1);
    QCOMPARE(y.at(0), QString("Garlic"));
    auto z = album->scoreTitles();
    QVERIFY(z.size() == 3);
    QCOMPARE(z.at(0), QString("Piano1"));
    QCOMPARE(z.at(1), QString("Piano2"));
    QCOMPARE(z.at(2), QString("Piano3"));

    album->removeScore(0);

    auto x2 = album->composers();
    QVERIFY(x2.size() == 1);
    QCOMPARE(x2.at(0), QString("Oregano"));
    auto y2 = album->lyricists();
    QVERIFY(y2.size() == 1);
    QCOMPARE(y2.at(0), QString("Garlic"));
    auto z2 = album->scoreTitles();
    QVERIFY(z2.size() == 2);
    QCOMPARE(z2.at(0), QString("Piano2"));
    QCOMPARE(z2.at(1), QString("Piano3"));

    delete album;
}

//---------------------------------------------------------
//   addRemoveTest
//---------------------------------------------------------

void TestAlbumsIO::addRemoveTest(const char* file)
{
    MScore::debugMode = true;
    Album* album = readAlbum(DIR + QString(file) + ".msca");
    QVERIFY(album);

    // load scores
    for (auto& item : album->albumItems()) {
        QString path = item->fileInfo.canonicalFilePath();
        MasterScore* score = readScoreAlbums(path);
        item->setScore(score);
    }

    QVERIFY(album->albumItems().size() == 3);
    MasterScore* ms = album->albumItems().at(1)->score;
    album->removeScore(ms);
    QVERIFY(album->albumItems().size() == 2);
    album->addScore(ms);
    QVERIFY(album->albumItems().size() == 3);

    delete album;
}

QTEST_MAIN(TestAlbumsIO)
#include "tst_albumsIO.moc"
