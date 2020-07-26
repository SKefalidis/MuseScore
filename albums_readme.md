Notes:

Mass changes:
 - Mass changes, Actions menu, used by both albums and groups of normal scores (multible tabs selected or opened scores)
 - Easy way to select multiple scores.
 - Actions:
    - Add - Replace instruments.
    - Change style.
    - Change composer, lyricist.
    - Change footers/headers.
    - Enable/disable (via filters?)

Split Score:
 - Easy way to split a score into an album

General:
 - Decouple albums and multi movement scores
 - Improve the automated tests
 - Album shortcuts
 - Album of albums
 - There could be composer and lyricist strings in more than one place
 - Export options, score mode, album mode, export relative path, export absolute path

 Refactor and Code Quality:
 - Cleanup the main branch

 Bugs:
 - Fix all the places where inActiveAlbum should be inActiveAlbum and the dominantScore is the one in the scoreview (e.g. the teleporting stuff, sequencer setScoreView)
 - The temporary score uses the style of the first movement (as expected) but changing it's style changes the style of the first movement and not of the temporary score

 Investigate:
 - Disabled score->doLayout in ScoreView::paintEvent, did I break something???
 - Closing tab without clicking (clicking changes MuseScore::cv to the current scoreview) crashes because it thinks that it should close a tab in the second tab bar (probably fixed)

 Features:
 - Albums in the start center?
 - Start center 'Open a Score...' to 'Open File...'?
 - Select elements from different scores in album-mode.
 - Score mode printing
