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

#ifndef ALBUM_H
#define ALBUM_H

namespace Ms {

class Album;
class MasterScore;
class XmlReader;
class XmlWriter;

using std::unique_ptr;

//---------------------------------------------------------
//   AlbumItem
//---------------------------------------------------------

class AlbumItem {

public:
    AlbumItem(Album& album);
    AlbumItem(Album& album, MasterScore* score, bool enabled = true);
    ~AlbumItem();

    void setEnabled(bool b);
    bool enabled() const;
    void setScore(MasterScore* score);
    void readAlbumItem(XmlReader& reader);
    void writeAlbumItem(XmlWriter& writer, bool absolutePathEnabled);

    Album& album;
    MasterScore* score      { nullptr }; // make reference? (probably can't cause I am not reading while loading)
    QFileInfo fileInfo      { "-" };

private:
    bool m_enabled          { true };
};

//---------------------------------------------------------
//   Album
//---------------------------------------------------------

class Album {

public:
    Album(){};
    void addAlbumItem(unique_ptr<AlbumItem> aItem);
    void addScore(MasterScore* score, bool enabled = true);
    void addSectionBreak(AlbumItem* aItem);
    void addSectionBreaks();
    void removeScore(MasterScore* score);
    void removeScore(int index);
    void swap(int indexA, int indexB);
    static bool scoreInActiveAlbum(MasterScore* score);
    MasterScore* getDominant();
    QStringList composers() const;
    QStringList lyricists() const;
    QStringList scoreTitles() const;

    bool loadFromFile(const QString& path);
    void readAlbum(XmlReader& reader);
    bool saveToFile(const QString &path, bool absolutePathEnabled = true);
    void writeAlbum(XmlWriter& writer, bool absolutePathEnabled);

    std::vector<AlbumItem*> albumItems() const;
    const QString& albumTitle() const;
    void setAlbumTitle(const QString& newTitle); // delete?
    const QFileInfo& fileInfo() const;
    bool generateContents() const;
    void setGenerateContents(bool enabled);
    int defaultPlaybackDelay() const;
    void setDefaultPlaybackDelay(int ms);

    static Album* activeAlbum;

private:
    std::vector<unique_ptr<AlbumItem>> m_albumItems {};
    QString m_albumTitle                            { "" };
    QFileInfo m_fileInfo                            {};

    bool m_generateContents                         { false };
    int m_defaultPlaybackDelay                      { 1000 };
};

}     // namespace Ms

#endif // ALBUM_H
