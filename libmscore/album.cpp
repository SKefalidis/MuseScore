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

#include "album.h"
#include "score.h"
#include "xml.h"
#include "musescoreCore.h"

namespace Ms {

//---------------------------------------------------------
//---------------------------------------------------------
//
//   AlbumItem
//
//---------------------------------------------------------
//---------------------------------------------------------

AlbumItem::AlbumItem(Album* album)
{
    this->album = album;
}

AlbumItem::AlbumItem(Album* album, MasterScore *score, bool enabled) : AlbumItem(album)
{
    std::cout << "New album item..." << std::endl;
    this->score = score;
    this->enabled = enabled;
    _fileInfo.setFile(score->importedFilePath());
    score->setPartOfActiveAlbum(true);
    std::cout << "album item created..." << std::endl;
}

AlbumItem::~AlbumItem()
{
    score->setPartOfActiveAlbum(false); // also called in ~AlbumManagerItem, FIXME
}

//---------------------------------------------------------
//   setEnabled
//---------------------------------------------------------

void AlbumItem::setEnabled(bool b)
{
    enabled = b;
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void AlbumItem::setScore(MasterScore *score)
{
    this->score = score;
}

//---------------------------------------------------------
//   readAlbumItem
//---------------------------------------------------------

void AlbumItem::readAlbumItem(XmlReader &reader)
{
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());
        if (tag == "name") {
            reader.readElementText();
        } else if (tag == "path") {
            _fileInfo.setFile(reader.readElementText());
        } else if (tag == "relativePath") {
            if (!_fileInfo.exists()) {
                QDir dir(album->_fileInfo.dir());
                QString relativePath = reader.readElementText();
                _fileInfo.setFile(dir, relativePath);
            } else {
                reader.readElementText();
            }
        } else if (tag == "enabled") {
            enabled = reader.readBool();
        } else {
            reader.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   saveAlbumItem
//---------------------------------------------------------


void AlbumItem::writeAlbumItem(XmlWriter &writer, bool absolutePathEnabled)
{
    writer.stag("Score");
    writer.tag("alias", "");
    if (absolutePathEnabled) {
        writer.tag("path", _fileInfo.absoluteFilePath());
    }
    QDir dir(album->_fileInfo.dir());
    QString relativePath = dir.relativeFilePath(_fileInfo.absoluteFilePath());
    writer.tag("relativePath", relativePath);
    writer.tag("enabled", enabled);
    writer.etag();
}

//---------------------------------------------------------
//---------------------------------------------------------
//
//   Album
//
//---------------------------------------------------------
//---------------------------------------------------------

Album* Album::activeAlbum = nullptr;

Album::Album()
{

}

//---------------------------------------------------------
//   addScore
//---------------------------------------------------------

void Album::addScore(MasterScore *score, bool enabled)
{
    if (!score) {
        std::cout << "There is no score to add to album..." << std::endl;
        return;
    }
    std::cout << "Adding score to album..." << std::endl;

    AlbumItem* albumScore = new AlbumItem(this, score, enabled);
    _albumItems.push_back(albumScore);
}

//---------------------------------------------------------
//   removeScore
//---------------------------------------------------------

MasterScore* Album::removeScore(MasterScore *score)
{
    return nullptr;
}

MasterScore* Album::removeScore(int index)
{
    _albumItems.erase(_albumItems.begin() + index);
    return nullptr;
}

//---------------------------------------------------------
//   swap
//---------------------------------------------------------

void Album::swap(int indexA, int indexB)
{
    std::swap(_albumItems.at(indexA), _albumItems.at(indexB));
}

//---------------------------------------------------------
//   scoreInAlbum
//---------------------------------------------------------

bool Album::scoreInActiveAlbum(MasterScore *score)
{
    if (!activeAlbum)
        return false;

    for (auto x : activeAlbum->_albumItems) {
        if (x->score == score) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   getDominant
//---------------------------------------------------------

MasterScore *Album::getDominant()
{
    return _albumItems.front()->score;
}

//---------------------------------------------------------
//   loadFromFile
//---------------------------------------------------------

bool Ms::Album::loadFromFile(const QString &path)
{
    std::cout << "Loading album from file..." << std::endl;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open filestream to open album: " << path << endl;
        return false;
    }

    _fileInfo.setFile(path);
    XmlReader reader(&f);
    reader.setDevice(&f);
    readAlbum(reader);
    f.close();
    return true;
}

//---------------------------------------------------------
//   readAlbum
//---------------------------------------------------------

void Album::readAlbum(XmlReader &reader)
{
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());
        if (tag == "name") {
            _albumTitle = reader.readElementText();
        } else if (tag == "Score") {
            std::cout << "new album item" << std::endl;
            AlbumItem* aItem = new AlbumItem(this);
            aItem->readAlbumItem(reader);
            _albumItems.push_back(aItem);
        }
    }
}

//---------------------------------------------------------
//   saveToFile
//---------------------------------------------------------

bool Album::saveToFile(const QString &path, bool absolutePathEnabled)
{
    std::cout << "Saving album to file..." << std::endl;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open filestream to save album: " << path << endl;
        return false;
    }

    _fileInfo.setFile(path);
    XmlWriter writer(nullptr, &f);
    writer.header();
    writer.stag(QStringLiteral("museScore version=\"" MSC_VERSION"\""));
    writeAlbum(writer, absolutePathEnabled);
    writer.etag();
    f.close();
    return true;
}

//---------------------------------------------------------
//   writeAlbum
//---------------------------------------------------------

void Album::writeAlbum(XmlWriter &writer, bool absolutePathEnabled)
{
    writer.stag("Album");
    writer.tag("name", _albumTitle);
    for (auto& aItem : _albumItems) {
        aItem->writeAlbumItem(writer, absolutePathEnabled);
    }
    writer.etag();
}

}     // namespace Ms
