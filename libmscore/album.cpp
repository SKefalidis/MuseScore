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
#include "part.h"
#include "excerpt.h"
#include "score.h"
#include "xml.h"
#include "musescoreCore.h"
#include "measure.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"

namespace Ms {
//---------------------------------------------------------
//---------------------------------------------------------
//
//   AlbumExcerpt
//
//---------------------------------------------------------
//---------------------------------------------------------

AlbumExcerpt::AlbumExcerpt(XmlReader& reader)
{
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());
        if (tag == "partIndex") {
            partIndices.push_back(reader.readInt());
        } else if (tag == "key") {
            int k = reader.readInt();
            reader.readNextStartElement();
            int v = reader.readInt();
            tracks.insert(k, v);
        } else if (tag == "title") {
            title = reader.readElementText();
        }
    }
}

//---------------------------------------------------------
//   writeAlbumExcerpt
//---------------------------------------------------------

void AlbumExcerpt::writeAlbumExcerpt(XmlWriter& writer) const
{
    writer.stag("Excerpt");
    writer.tag("title", title);
    for (auto partIndex : partIndices) {
        writer.tag("partIndex", partIndex);
    }
    for (auto k : tracks.uniqueKeys()) {
        writer.tag("key", k);
        for (auto v : tracks.values(k)) {
            writer.tag("track", v);
        }
    }
    writer.etag();
}

//---------------------------------------------------------
//---------------------------------------------------------
//
//   AlbumItem
//
//---------------------------------------------------------
//---------------------------------------------------------

AlbumItem::AlbumItem(Album& album, XmlReader& reader)
    : album(album)
{
    readAlbumItem(reader);
}

AlbumItem::AlbumItem(Album& album, MasterScore* score, bool enabled)
    : album(album)
{
    m_enabled = enabled;
    setScore(score);
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
    if (album.getDominant()) {
        album.getDominant()->update();
        album.getDominant()->doLayout();
    }
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
        qDebug() << "The AlbumItem already has a Score, don't set a new one. Create a new AlbumItem." << endl;
//        Q_ASSERT(false);
        return -1;
    }
    // don't set an empty score
    if (score == nullptr) {
        qDebug() << "You are trying to set an empty score." << endl;
//        Q_ASSERT(false);
        return -1;
    }

    this->score = score;
    setEnabled(m_enabled);
    score->setPartOfActiveAlbum(true);
    fileInfo.setFile(score->importedFilePath());

    addSectionBreak();
    if (album.addPageBreaksEnabled()) {
        addPageBreak();
    }

    updateDuration();
    connect(score, &MasterScore::durationChanged, this, &AlbumItem::updateDuration);
    return 0;
}

//---------------------------------------------------------
//   addSectionBreak
//---------------------------------------------------------

void AlbumItem::addSectionBreak()
{
    if (!score->lastMeasure()->sectionBreak()) { // add only if there isn't one
        LayoutBreak* lb = new LayoutBreak(score);
        lb->setLayoutBreakType(LayoutBreak::Type::SECTION);
        score->lastMeasure()->add(lb);
        score->update();
    }
}

//---------------------------------------------------------
//   addPageBreak
//---------------------------------------------------------

void AlbumItem::addPageBreak()
{
    if (!score->lastMeasure()->pageBreak()) { // add only if there isn't one
        LayoutBreak* lb = new LayoutBreak(score);
        lb->setLayoutBreakType(LayoutBreak::Type::PAGE);
        score->lastMeasure()->add(lb);
        score->update();
    }
}

//---------------------------------------------------------
//   readAlbumItem
//---------------------------------------------------------

void AlbumItem::readAlbumItem(XmlReader& reader)
{
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());
        if (tag == "name") {
            reader.readElementText();
        } else if (tag == "path") {
            fileInfo.setFile(reader.readElementText());
            album.setIncludeAbsolutePaths(true);
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

void AlbumItem::writeAlbumItem(XmlWriter& writer) const
{
    writer.stag("Score");
    writer.tag("alias", "");
    if (album.includeAbsolutePaths()) {
        writer.tag("path", fileInfo.absoluteFilePath());
    }
    if (!album.exporting()) {
        QDir dir(album.fileInfo().dir());
        QString relativePath = dir.relativeFilePath(fileInfo.absoluteFilePath());
        writer.tag("relativePath", relativePath);
    } else {
        writer.tag("relativePath", album.exportedScoreFolder() + QDir::separator() + score->title() + ".mscx");
    }
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
//   createItem
//---------------------------------------------------------

AlbumItem* Album::createItem(XmlReader& reader)
{
    AlbumItem* a = new AlbumItem(*this, reader);
    m_albumItems.push_back(std::unique_ptr<AlbumItem>(a));
    return a;
}

AlbumItem* Album::createItem(MasterScore* score, bool enabled)
{
    AlbumItem* a = new AlbumItem(*this, score, enabled);
    m_albumItems.push_back(std::unique_ptr<AlbumItem>(a));
    return a;
}

//---------------------------------------------------------
//   addScore
//---------------------------------------------------------

AlbumItem* Album::addScore(MasterScore* score, bool enabled)
{
    if (!score) {
        std::cout << "There is no score to add to album..." << std::endl;
        return nullptr;
    }
    if (m_dominantScore && m_dominantScore->excerpts().size() > 0) {
        int partCount = m_dominantScore->parts().size();
        for (int i = 0; i < partCount; i++) {
            for (auto x : *m_dominantScore->movements()) {
                if (x->score()->parts().at(i)->partName().compare(score->parts().at(i)->partName(), Qt::CaseSensitivity::CaseInsensitive)) {
                    std::cout << "Parts not matching..." << std::endl;
                    QMessageBox msgBox;
                    msgBox.setWindowTitle(QObject::tr("Incompatible parts"));
                    msgBox.setText(QString("The parts of your new score are incompatible with the rest of the album."));
                    msgBox.setDetailedText(QString("The parts of your new score are incompatible with the rest of the album. That means "
                                                   "that adding this score will break the `Parts` functionality for your Album. You can"
                                                   "remove this score to restore this functionality. "));
                    msgBox.setTextFormat(Qt::RichText);
                    msgBox.setIcon(QMessageBox::Warning);
                    msgBox.setStandardButtons(
                        QMessageBox::Cancel | QMessageBox::Ignore
                        );
                    auto response = msgBox.exec();
                    if (response == QMessageBox::Cancel) {
                        return nullptr;
                    } else {
                        while (m_dominantScore->excerpts().size()) {
                            m_dominantScore->removeExcerpt(m_dominantScore->excerpts().first());
                        }
                        goto exit_loop;
                    }
                }
            }
        }
    }
exit_loop:;
    std::cout << "Adding score to album..." << std::endl;
    AlbumItem* a = createItem(score, enabled);

    if (m_dominantScore) {
        m_dominantScore->addMovement(score);
        m_dominantScore->update();
        m_dominantScore->doLayout(); // position the movements correctly
        if (m_dominantScore->excerpts().size() > 0) {
            // add movement to excerpts
            for (auto& e : m_dominantScore->excerpts()) {
                Excerpt* ee = createMovementExcerpt(prepareMovementExcerpt(e, score));
                static_cast<MasterScore*>(e->partScore())->addMovement(static_cast<MasterScore*>(ee->partScore()));
            }
        }
    }
    return a;
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
        // remove the movement from the dominantScore
        if (m_dominantScore->excerpts().size() > 0) {
            // remove movement from excerpts
            for (auto& e : m_dominantScore->excerpts()) {
                static_cast<MasterScore*>(e->partScore())->removeMovement(index + 1);
            }
        }
        m_dominantScore->removeMovement(index + 1);
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
    if (m_dominantScore) {
        int offset = m_dominantScore->firstRealMovement();
        std::swap(m_dominantScore->movements()->at(indexA + offset), m_dominantScore->movements()->at(indexB + offset));
        m_dominantScore->doLayout(); // position the movements correctly
    }
}

//---------------------------------------------------------
//   addSectionBreaks
//---------------------------------------------------------

void Album::addSectionBreaks()
{
    for (auto& aItem : m_albumItems) {
        aItem->addSectionBreak();
    }
}

//---------------------------------------------------------
//   addPageBreaks
//---------------------------------------------------------

void Album::addPageBreaks()
{
    for (auto& aItem : m_albumItems) {
        aItem->addPageBreak();
    }
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
//   scoreInAlbum
//---------------------------------------------------------

bool Album::scoreInActiveAlbum(MasterScore* score)
{
    if (!activeAlbum) {
        return false;
    }

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

MasterScore* Album::getDominant() const
{
    return m_dominantScore;
}

//---------------------------------------------------------
//   setDominant
//---------------------------------------------------------

void Album::setDominant(MasterScore* ms)
{
    if (m_dominantScore) {
        m_dominantScore->setPartOfActiveAlbum(false);
        // TODO_SK: remove the score from the scoreList and delete it
    }
    ms->setPartOfActiveAlbum(true);
    m_dominantScore = ms;
    // set Parts
    if (m_dominantScore->excerpts().isEmpty()) {
        for (auto& e : m_albumExcerpts) {
            //
            // prepare Excerpts
            //
            Excerpt* ne = new Excerpt(m_dominantScore);
            ne->setTitle(e->title);
            for (auto partIndex : e->partIndices) {
                ne->parts().append(m_dominantScore->parts().at(partIndex));
            }
            ne->setTracks(e->tracks);
            //
            // create Excerpts
            //
            MasterScore* nscore = new MasterScore(ne->oscore());
            ne->setPartScore(nscore);
            nscore->setName(ne->oscore()->title() + "_part_" + ne->oscore()->excerpts().size());
            m_dominantScore->addExcerpt(ne);
            Excerpt::createExcerpt(ne);

            // a new excerpt is created in AddExcerpt, make sure the parts are filed
            for (Excerpt* ee : ne->oscore()->excerpts()) {
                if (ee->partScore() == nscore && ee != ne) {
                    ee->parts().clear();
                    ee->parts().append(ne->parts());
                }
            }

            for (auto m : *m_dominantScore->movements()) {
                if (m == m_dominantScore) {
                    continue;
                }
                Excerpt* ee = createMovementExcerpt(prepareMovementExcerpt(ne, m));
                nscore->addMovement(static_cast<MasterScore*>(ee->partScore()));
            }

            nscore->setLayoutAll();
            nscore->setUpdateAll();
            nscore->undoChangeStyleVal(MSQE_Sid::Sid::spatium, 25.016); // hack: normally it's 25 but it draws crazy stuff with that
            nscore->update();
        }
    }
}

//---------------------------------------------------------
//   loadFromFile
//---------------------------------------------------------

bool Album::loadFromFile(const QString& path)
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
    readExcerpts(reader);
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
            createItem(reader);
        } else if (tag == "generateContents") {
            m_generateContents = reader.readBool();
        } else if (tag == "addPageBreaks") {
            m_addPageBreaksEnabled = reader.readBool();
        } else if (tag == "titleAtTheBottom") {
            m_titleAtTheBottom = reader.readBool();
        } else if (tag == "playbackDelay") {
            m_defaultPlaybackDelay = reader.readInt();
        }
    }
}

//---------------------------------------------------------
//   readExcerpts
//---------------------------------------------------------

void Album::readExcerpts(XmlReader& reader)
{
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());
        if (tag == "Excerpt") {
            m_albumExcerpts.push_back(std::unique_ptr<AlbumExcerpt>(new AlbumExcerpt(reader)));
        }
    }
}

//---------------------------------------------------------
//   saveToFile
//---------------------------------------------------------

bool Album::saveToFile(const QString& path)
{
    std::cout << "Saving album to file..." << std::endl;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open filestream to save album: " << path << endl;
        return false;
    }

    m_fileInfo.setFile(path);
    saveToFile(&f);
    f.close();
    return true;
}

// used for exporting
bool Album::saveToFile(QIODevice* f)
{
    bool b = m_includeAbsolutePaths;
    m_includeAbsolutePaths = false;
    m_exporting = true;

    XmlWriter writer(nullptr, f);
    writer.header();
    writer.stag(QStringLiteral("museScore version=\"" MSC_VERSION "\""));
    writeAlbum(writer);
    writer.etag();

    m_includeAbsolutePaths = b;
    m_exporting = false;
    return true;
}

//---------------------------------------------------------
//   writeAlbum
//---------------------------------------------------------

void Album::writeAlbum(XmlWriter& writer) const
{
    writer.stag("Album");
    writer.tag("name", m_albumTitle);
    writer.tag("generateContents", m_generateContents);
    writer.tag("addPageBreaks", m_addPageBreaksEnabled);
    writer.tag("titleAtTheBottom", m_titleAtTheBottom);
    writer.tag("playbackDelay", m_defaultPlaybackDelay);
    for (auto& aItem : m_albumItems) {
        aItem->writeAlbumItem(writer);
    }
    writer.etag();
    writer.stag("Excerpts");
    if (m_dominantScore) {
        for (auto e : m_dominantScore->excerpts()) {
            e->writeForAlbum(writer);
        }
    } else {
        for (auto& e : m_albumExcerpts) {
            e->writeAlbumExcerpt(writer);
        }
    }
    writer.etag();
}

//---------------------------------------------------------
//   readRootFile
//---------------------------------------------------------

QString readRootFile(MQZipReader* uz)
{
    QString rootfile;

    QByteArray cbuf = uz->fileData("META-INF/container.xml");
    if (cbuf.isEmpty()) {
        qDebug("can't find container.xml");
        return rootfile;
    }

    XmlReader e(cbuf);

    while (e.readNextStartElement()) {
        if (e.name() != "container") {
            e.unknown();
            continue;
        }
        while (e.readNextStartElement()) {
            if (e.name() != "rootfiles") {
                e.unknown();
                continue;
            }
            while (e.readNextStartElement()) {
                const QStringRef& tag(e.name());

                if (tag == "rootfile") {
                    if (rootfile.isEmpty()) {
                        rootfile = e.attribute("full-path");
                        e.skipCurrentElement();
                    }
                } else {
                    e.unknown();
                }
            }
        }
    }
    return rootfile;
}

//---------------------------------------------------------
//   importAlbum
//---------------------------------------------------------

void Album::importAlbum(const QString& compressedFilePath, QDir destinationFolder)
{
    destinationFolder.mkdir("Scores");
    MQZipReader uz(compressedFilePath, QIODevice::ReadWrite);
    uz.extractAll(destinationFolder.path());
}

//---------------------------------------------------------
//   exportAlbum
//---------------------------------------------------------

bool Album::exportAlbum(QIODevice* f, const QFileInfo& info)
{
    MQZipWriter uz(f);
    QBuffer dbuf;

    QString fn = info.completeBaseName() + ".msca";
    dbuf.open(QIODevice::ReadWrite);
    saveToFile(&dbuf);
    dbuf.seek(0);
    uz.addFile(fn, dbuf.data());
    dbuf.close();

    QFileDevice* fd = dynamic_cast<QFileDevice*>(f);
    if (fd) { // if is file (may be buffer)
        fd->flush();     // flush to preserve score data in case of
    }
    // any failures on the further operations.

    for (auto x : albumItems()) {
        QString path = m_exportedScoreFolder + QDir::separator() + x->score->title() + ".mscx";
        dbuf.open(QIODevice::ReadWrite);
        x->score->Score::saveFile(&dbuf, false);
        dbuf.seek(0);
        uz.addFile(path, dbuf.data());
        dbuf.close();
        fd->flush();
    }

    fd->flush();
    uz.close();
    return true;
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
    std::vector<AlbumItem*> ai {};
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
//   includeAbsolutePaths
//---------------------------------------------------------

bool Album::includeAbsolutePaths() const
{
    return m_includeAbsolutePaths;
}

//---------------------------------------------------------
//   setIncludeAbsolutePaths
//---------------------------------------------------------

void Album::setIncludeAbsolutePaths(bool enabled)
{
    m_includeAbsolutePaths = enabled;
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

//---------------------------------------------------------
//   exportedScoreFolder
//---------------------------------------------------------

const QString& Album::exportedScoreFolder() const
{
    return m_exportedScoreFolder;
}

//---------------------------------------------------------
//   exporting
//---------------------------------------------------------

bool Album::exporting() const
{
    return m_exporting;
}
}     // namespace Ms
