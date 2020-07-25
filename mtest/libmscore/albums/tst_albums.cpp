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

    void albumItemConstructorWithScoreTest();
    void albumItemConstructorWithoutScoreTest();
    void albumItemTest(Album& myAlbum, AlbumItem& aItem, MasterScore& aScore);

private slots:
    void initTestCase();

    void albumItemWithScore() { albumItemConstructorWithScoreTest(); };
    void albumItemWithoutScore() { albumItemConstructorWithoutScoreTest(); };
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestAlbums::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   albumItemConstructorWithScoreTest
//---------------------------------------------------------

void TestAlbums::albumItemConstructorWithScoreTest()
{
    Album myAlbum;
    MasterScore* aScore = readScore(DIR + "AlbumItemTest.mscx");
    AlbumItem* aItem = new AlbumItem(myAlbum, aScore, true);

    QCOMPARE(&aItem->album, &myAlbum);
    QVERIFY(myAlbum.albumItems().size() == 1);

    albumItemTest(myAlbum, *aItem, *aScore);
}

//---------------------------------------------------------
//   albumItemConstructorWithoutScoreTest
//---------------------------------------------------------

void TestAlbums::albumItemConstructorWithoutScoreTest()
{
    Album myAlbum;
    MasterScore* aScore = readScore(DIR + "AlbumItemTest.mscx");
    AlbumItem* aItem = new AlbumItem(myAlbum, aScore, true);

    QCOMPARE(&aItem->album, &myAlbum);
    QVERIFY(myAlbum.albumItems().size() == 1);

    // without score
    QVERIFY(!aScore->partOfActiveAlbum());
    QCOMPARE(aItem->duration(), -1);

    QCOMPARE(aItem->setScore(aScore), 0);

    albumItemTest(myAlbum, *aItem, *aScore);
    delete aScore;
}

//---------------------------------------------------------
//   albumItemTest
//---------------------------------------------------------

void TestAlbums::albumItemTest(Album& myAlbum, AlbumItem& aItem, MasterScore& aScore)
{
    MasterScore* bScore = new MasterScore();

    QVERIFY(aScore.partOfActiveAlbum());

    QCOMPARE(aItem.duration(), aScore.duration());
    int scoreDuration1 = aScore.duration();
    aScore.appendMeasures(2);
    int scoreDuration2 = aScore.duration();
    QVERIFY(scoreDuration1 < scoreDuration2);
    QCOMPARE(aItem.duration(), aScore.duration());

    QVERIFY(aItem.enabled());
    QCOMPARE(aItem.enabled(), aScore.enabled()); // true
    aItem.setEnabled(false);
    QVERIFY(!aItem.enabled());
    QCOMPARE(aItem.enabled(), aScore.enabled()); // false

    QCOMPARE(aItem.setScore(bScore), -1);

    delete bScore;
}

QTEST_MAIN(TestAlbums)
#include "tst_albums.moc"
