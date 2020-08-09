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
class Excerpt;
enum class LayoutMode : char;

using std::unique_ptr;

//---------------------------------------------------------
//   AlbumExcerpt
//---------------------------------------------------------

struct AlbumExcerpt
{
    AlbumExcerpt(XmlReader& reader);
    void writeAlbumExcerpt(XmlWriter& writer) const;

    QString title;
    QList<int> partIndices;
    QMultiMap<int, int> tracks;
};

//---------------------------------------------------------
//   AlbumItem
//---------------------------------------------------------

class AlbumItem : public QObject
{
    Q_OBJECT

public:
    // AlbumItems are created by Album::createItem, don't create them manually.
    AlbumItem(Album& album, XmlReader& reader);
    AlbumItem(Album& album, MasterScore* score, bool enabled = true);
    ~AlbumItem();

    void setEnabled(bool b);
    bool enabled() const;
    int setScore(MasterScore* score);
    void addSectionBreak();
    void addPageBreak();
    void readAlbumItem(XmlReader& reader);
    void writeAlbumItem(XmlWriter& writer) const;

    int duration() const;

    Album& album;
    MasterScore* score      { nullptr }; // make reference? (probably can't cause I am not reading while loading)
    QFileInfo fileInfo      { "-" };

signals:
    void durationChanged();

private slots:
    void updateDuration();

private:
    bool m_enabled { true };
    int m_duration { -1 };
};

//---------------------------------------------------------
//   Album
//---------------------------------------------------------

class Album : public QObject
{
    Q_OBJECT

public:
    static Album* activeAlbum;
    static bool scoreInActiveAlbum(MasterScore* score);

    AlbumItem* addScore(MasterScore* score, bool enabled = true);
    void removeScore(MasterScore* score);
    void removeScore(int index);
    void swap(int indexA, int indexB);

    void addSectionBreaks();
    void addPageBreaks();

    QStringList composers() const;
    QStringList lyricists() const;
    QStringList scoreTitles() const;

    bool loadFromFile(const QString& path);
    void readAlbum(XmlReader& reader);
    void readExcerpts(XmlReader& reader);
    bool saveToFile(const QString& path);
    bool saveToFile(QIODevice* f);
    void writeAlbum(XmlWriter& writer) const;
    bool exportAlbum(QIODevice* f, const QFileInfo& info);

    MasterScore* getDominant() const;
    void setDominant(MasterScore* ms); // I don't like this function.
    std::vector<AlbumItem*> albumItems() const;

    static Excerpt* prepareMovementExcerpt(Excerpt* masterExcerpt, MasterScore* score);
    static Excerpt* createMovementExcerpt(Excerpt*);

    const QString& albumTitle() const;
    void setAlbumTitle(const QString& newTitle);
    const QFileInfo& fileInfo() const;
    bool albumModeActive() const;
    void setAlbumModeActive(bool b);
    bool titleAtTheBottom() const;
    void setTitleAtTheBottom(bool titleAtTheBottom);
    bool generateContents() const;
    void setGenerateContents(bool enabled);
    bool addPageBreaksEnabled() const;
    void setAddPageBreaksEnabled(bool enabled);
    bool includeAbsolutePaths() const;
    void setIncludeAbsolutePaths(bool enabled);
    int defaultPlaybackDelay() const;
    void setDefaultPlaybackDelay(int ms);

public slots:
    void setAlbumLayoutMode(LayoutMode lm);

private:
    AlbumItem* createItem(XmlReader& reader);
    AlbumItem* createItem(MasterScore* score, bool enabled);

    std::vector<unique_ptr<AlbumItem> > m_albumItems {};
    std::vector<unique_ptr<AlbumExcerpt> > m_albumExcerpts {};
    QString m_albumTitle                            { "" };
    QFileInfo m_fileInfo                            {};
    MasterScore* m_dominantScore                    { nullptr };
    bool m_albumModeActive                          { false };

    bool m_titleAtTheBottom                         { true };
    bool m_generateContents                         { false };
    bool m_addPageBreaksEnabled                     { false };
    bool m_includeAbsolutePaths                     { false };
    int m_defaultPlaybackDelay                      { 3000 };
};
}     // namespace Ms

#endif // ALBUM_H
