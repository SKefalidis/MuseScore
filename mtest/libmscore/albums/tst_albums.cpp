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

    void albumAddScore();

private slots:
    void initTestCase();

    void albumAddScoreTest() { albumAddScore(); }
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestAlbums::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   albumItemTest
//---------------------------------------------------------

void TestAlbums::albumAddScore()
{
    Album myAlbum;
    MasterScore* aScore = readScore(DIR + "AlbumItemTest.mscx");
    MasterScore* bScore = new MasterScore();
    AlbumItem* aItem = myAlbum.addScore(aScore, true);

    QCOMPARE(&aItem->album, &myAlbum);
    QVERIFY(myAlbum.albumItems().size() == 1);

    QVERIFY(aScore->partOfActiveAlbum());

    QCOMPARE(aItem->duration(), aScore->duration());
    int scoreDuration1 = aScore->duration();
    aScore->appendMeasures(2);
    int scoreDuration2 = aScore->duration();
    QVERIFY(scoreDuration1 < scoreDuration2);
    QCOMPARE(aItem->duration(), aScore->duration());

    QVERIFY(aItem->enabled());
    QCOMPARE(aItem->enabled(), aScore->enabled()); // true
    aItem->setEnabled(false);
    QVERIFY(!aItem->enabled());
    QCOMPARE(aItem->enabled(), aScore->enabled()); // false

    QCOMPARE(aItem->setScore(bScore), -1);

    delete bScore;
}

QTEST_MAIN(TestAlbums)
#include "tst_albums.moc"
