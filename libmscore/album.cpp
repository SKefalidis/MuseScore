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
#include "measure.h"

namespace Ms {

//---------------------------------------------------------
//---------------------------------------------------------
//
//   AlbumItem
//
//---------------------------------------------------------
//---------------------------------------------------------

AlbumItem::AlbumItem(Album& album)
    : album(album)
{
    album.addAlbumItem(unique_ptr<AlbumItem>(this));
}

AlbumItem::AlbumItem(Album& album, XmlReader& reader)
    : AlbumItem(album)
{
    readAlbumItem(reader);
}

AlbumItem::AlbumItem(Album& album, MasterScore *score, bool enabled)
    : AlbumItem(album)
{
    this->score = score;
    setEnabled(enabled);
    score->setPartOfActiveAlbum(true);
    updateDuration();
    fileInfo.setFile(score->importedFilePath());
}

AlbumItem::~AlbumItem()
{
    if (score) {
        score->setPartOfActiveAlbum(false); // also called in ~AlbumManagerItem, FIXME
    }
    // TODO_SK: if (score not in score list, delete it)
}

//---------------------------------------------------------
//   setEnabled
//---------------------------------------------------------

void AlbumItem::setEnabled(bool b)
{
    m_enabled = b;
    score->setEnabled(b);
}

bool AlbumItem::enabled() const
{
    return m_enabled;
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

int AlbumItem::setScore(MasterScore* score)
{
    // if you want to change the score, create a new AlbumItem
    if (this->score != nullptr) {
        Q_ASSERT(false);
        return -1;
    }
    // don't set an empty score
    if (score == nullptr) {
        Q_ASSERT(false);
        return -1;
    }

    disconnect(score, &MasterScore::durationChanged, this, &AlbumItem::updateDuration);
    this->score = score;
    setEnabled(m_enabled);
    score->setPartOfActiveAlbum(true);
    updateDuration();
    connect(score, &MasterScore::durationChanged, this, &AlbumItem::updateDuration);
    return 0;
}

//---------------------------------------------------------
//   readAlbumItem
//---------------------------------------------------------

void AlbumItem::readAlbumItem(XmlReader& reader)
{
    std::cout << "reading album item" << std::endl;
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());
        if (tag == "name") {
            reader.readElementText();
        } else if (tag == "path") {
            fileInfo.setFile(reader.readElementText());
        } else if (tag == "relativePath") {
            if (!fileInfo.exists()) {
                QDir dir(album.fileInfo().dir());
                QString relativePath = reader.readElementText();
                fileInfo.setFile(dir, relativePath);
            } else {
                reader.readElementText();
            }
        } else if (tag == "enabled") {
            m_enabled = reader.readBool();
        } else {
            reader.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   saveAlbumItem
//---------------------------------------------------------


void AlbumItem::writeAlbumItem(XmlWriter& writer, bool absolutePathEnabled) const
{
    writer.stag("Score");
    writer.tag("alias", "");
    if (absolutePathEnabled) {
        writer.tag("path", fileInfo.absoluteFilePath());
    }
    QDir dir(album.fileInfo().dir());
    QString relativePath = dir.relativeFilePath(fileInfo.absoluteFilePath());
    writer.tag("relativePath", relativePath);
    writer.tag("enabled", m_enabled);
    writer.etag();
}

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

int AlbumItem::duration() const
{
    return m_duration;
}

//---------------------------------------------------------
//   updateDuration
//---------------------------------------------------------

void AlbumItem::updateDuration()
{
    m_duration = score->duration();
    emit durationChanged();
}

//---------------------------------------------------------
//---------------------------------------------------------
//
//   Album
//
//---------------------------------------------------------
//---------------------------------------------------------

Album* Album::activeAlbum = nullptr;

//---------------------------------------------------------
//   addAlbumItem
//---------------------------------------------------------

void Album::addAlbumItem(unique_ptr<AlbumItem> aItem)
{
    // add section break to the end of the movement (if one does not already exist)
    if (aItem->score) {
        addSectionBreak(aItem.get());
    }
    if (aItem->score && m_addPageBreaksEnabled) {
        addPageBreak(aItem.get());
    }
    m_albumItems.push_back(std::move(aItem));
}

//---------------------------------------------------------
//   addScore
//---------------------------------------------------------

void Album::addScore(MasterScore* score, bool enabled)
{
    if (!score) {
        std::cout << "There is no score to add to album..." << std::endl;
        return;
    }
    std::cout << "Adding score to album..." << std::endl;
    new AlbumItem(*this, score, enabled);
}

//---------------------------------------------------------
//   addSectionBreak
//---------------------------------------------------------

void Album::addSectionBreak(AlbumItem* aItem)
{
    if (!aItem->score->lastMeasure()->sectionBreak()) { // add only if there isn't one
        LayoutBreak* lb = new LayoutBreak(aItem->score);
        lb->setLayoutBreakType(LayoutBreak::Type::SECTION);
        aItem->score->lastMeasure()->add(lb);
        aItem->score->update();
    }
}

//---------------------------------------------------------
//   addSectionBreaks
//---------------------------------------------------------

void Album::addSectionBreaks()
{
    for (auto& aItem : m_albumItems) {
        addSectionBreak(aItem.get());
    }
}

//---------------------------------------------------------
//   addPageBreak
//---------------------------------------------------------

void Album::addPageBreak(AlbumItem* aItem)
{
    if (!aItem->score->lastMeasure()->pageBreak()) { // add only if there isn't one
        LayoutBreak* lb = new LayoutBreak(aItem->score);
        lb->setLayoutBreakType(LayoutBreak::Type::PAGE);
        aItem->score->lastMeasure()->add(lb);
        aItem->score->update();
    }
}

//---------------------------------------------------------
//   addPageBreaks
//---------------------------------------------------------

void Album::addPageBreaks()
{
    for (auto& aItem : m_albumItems) {
        addPageBreak(aItem.get());
    }
}

//---------------------------------------------------------
//   removeScore
//---------------------------------------------------------

void Album::removeScore(MasterScore* score)
{
    for (int i = 0; i < int(m_albumItems.size()); i++) {
        if (m_albumItems.at(i)->score == score) {
            removeScore(i);
            break;
        }
    }
}

void Album::removeScore(int index)
{
    if (m_dominantScore) {
        m_dominantScore->removeMovement(m_albumItems.at(index)->score);
    }
    m_albumItems.at(index)->score->setPartOfActiveAlbum(false);
    m_albumItems.erase(m_albumItems.begin() + index);
}

//---------------------------------------------------------
//   swap
//---------------------------------------------------------

void Album::swap(int indexA, int indexB)
{
    std::swap(m_albumItems.at(indexA), m_albumItems.at(indexB));
}

//---------------------------------------------------------
//   scoreInAlbum
//---------------------------------------------------------

bool Album::scoreInActiveAlbum(MasterScore* score)
{
    if (!activeAlbum)
        return false;

    for (auto& x : activeAlbum->m_albumItems) {
        if (x->score == score) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   getDominant
//---------------------------------------------------------

MasterScore *Album::getDominant() const
{
    return m_dominantScore;
}

//---------------------------------------------------------
//   setDominant
//---------------------------------------------------------

void Album::setDominant(MasterScore* ms)
{
    m_dominantScore = ms;
}

//---------------------------------------------------------
//   composers
//---------------------------------------------------------

QStringList Album::composers() const
{
    QStringList composers;

    for (auto& item : m_albumItems) {
        QString composer = item->score->composer();
        if (!composers.contains(composer, Qt::CaseSensitivity::CaseInsensitive) && !composer.isEmpty()) {
            composers.push_back(composer);
        }
    }

    return composers;
}

//---------------------------------------------------------
//   lyricists
//---------------------------------------------------------

QStringList Album::lyricists() const
{
    QStringList lyricists;

    for (auto& item : m_albumItems) {
        QString lyricist = item->score->lyricist();
        if (!lyricists.contains(lyricist) && !lyricist.isEmpty()) {
            lyricists.push_back(lyricist);
        }
    }

    return lyricists;
}

//---------------------------------------------------------
//   scoreTitles
//---------------------------------------------------------

QStringList Album::scoreTitles() const
{
    QStringList scoreTitles;

    for (auto& item : m_albumItems) {
        if (!item->score->emptyMovement()) {
            QString title = item->score->realTitle();
            title = title.isEmpty() ? item->score->title() : title;
            scoreTitles.push_back(title);
        }
    }

    return scoreTitles;
}

//---------------------------------------------------------
//   loadFromFile
//---------------------------------------------------------

bool Album::loadFromFile(const QString &path)
{
    std::cout << "Loading album from file..." << std::endl;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open filestream to open album: " << path << endl;
        return false;
    }

    m_fileInfo.setFile(path);
    XmlReader reader(&f);
    reader.setDevice(&f);
    readAlbum(reader);
    f.close();
    return true;
}

//---------------------------------------------------------
//   readAlbum
//---------------------------------------------------------

void Album::readAlbum(XmlReader& reader)
{
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());
        if (tag == "name") {
            m_albumTitle = reader.readElementText();
        } else if (tag == "Score") {
            new AlbumItem(*this, reader);
        } else if (tag == "generateContents") {
            m_generateContents = reader.readBool();
        } else if (tag == "addPageBreaks") {
            m_addPageBreaksEnabled = reader.readBool();
        } else if (tag == "playbackDelay") {
            m_defaultPlaybackDelay = reader.readInt();
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

    m_fileInfo.setFile(path);
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

void Album::writeAlbum(XmlWriter &writer, bool absolutePathEnabled) const
{
    writer.stag("Album");
    writer.tag("name", m_albumTitle);
    writer.tag("generateContents", m_generateContents);
    writer.tag("addPageBreaks", m_addPageBreaksEnabled);
    writer.tag("playbackDelay", m_defaultPlaybackDelay);
    for (auto& aItem : m_albumItems) {
        aItem->writeAlbumItem(writer, absolutePathEnabled);
    }
    writer.etag();
}

//---------------------------------------------------------
//   setAlbumLayoutMode
//---------------------------------------------------------

void Album::setAlbumLayoutMode(LayoutMode lm)
{
    for (auto& x : m_albumItems) {
        x->score->setLayoutMode(lm);
        x->score->doLayout();
    }
    if (m_dominantScore) {
        m_dominantScore->doLayout();
    }
}

//---------------------------------------------------------
//   albumItems
//---------------------------------------------------------

std::vector<AlbumItem*> Album::albumItems() const
{
    std::vector<AlbumItem*> ai;
    for (auto& x : m_albumItems) {
        ai.push_back(x.get());
    }
    return ai;
}

//---------------------------------------------------------
//   albumTitle
//---------------------------------------------------------

const QString& Album::albumTitle() const
{
    return m_albumTitle;
}

//---------------------------------------------------------
//   setAlbumTitle
//---------------------------------------------------------

void Album::setAlbumTitle(const QString& newTitle)
{
    m_albumTitle = newTitle;
}

//---------------------------------------------------------
//   fileInfo
//---------------------------------------------------------

const QFileInfo& Album::fileInfo() const
{
    return m_fileInfo;
}

//---------------------------------------------------------
//   albumModeActive
//---------------------------------------------------------


bool Album::albumModeActive() const
{
    return m_albumModeActive;
}


//---------------------------------------------------------
//   setAlbumModeActive
//---------------------------------------------------------

void Album::setAlbumModeActive(bool b)
{
    m_albumModeActive = b;
}

//---------------------------------------------------------
//   titleAtTheBottom
//---------------------------------------------------------

bool Album::titleAtTheBottom() const
{
    return m_titleAtTheBottom;
}

//---------------------------------------------------------
//   setTitleAtTheBottom
//---------------------------------------------------------

void Album::setTitleAtTheBottom(bool titleAtTheBottom)
{
    m_titleAtTheBottom = titleAtTheBottom;
    if (m_dominantScore) {
        m_dominantScore->setTitleAtTheBottom(titleAtTheBottom);
    }
}

//---------------------------------------------------------
//   generateContents
//---------------------------------------------------------

bool Album::generateContents() const
{
    return m_generateContents;
}

//---------------------------------------------------------
//   setGenerateContents
//---------------------------------------------------------

void Album::setGenerateContents(bool enabled)
{
    m_generateContents = enabled;
}

//---------------------------------------------------------
//   addPageBreaksEnabled
//---------------------------------------------------------

bool Album::addPageBreaksEnabled() const
{
    return m_addPageBreaksEnabled;
}

//---------------------------------------------------------
//   setAddPageBreaksEnabled
//---------------------------------------------------------

void Album::setAddPageBreaksEnabled(bool enabled)
{
    m_addPageBreaksEnabled = enabled;
}

//---------------------------------------------------------
//   defaultPlaybackDelay
//---------------------------------------------------------

int Album::defaultPlaybackDelay() const
{
    return m_defaultPlaybackDelay;
}

//---------------------------------------------------------
//   setDefaultPlaybackDelay
//---------------------------------------------------------

void Album::setDefaultPlaybackDelay(int ms)
{
    m_defaultPlaybackDelay = ms;
}

}     // namespace Ms
