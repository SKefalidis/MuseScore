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
 - Album shortcuts
 - Undo
 - Preferences
     - Export options, export relative path, export absolute path
 - Album of albums
 - Parts???

 Refactor and Code Quality:
 - Cleanup the main branch
 - Codestyle
 - Improve the automated tests
 - Decouple albums and multi movement scores

 Bugs:
 - Fix all the places where inActiveAlbum should be inActiveAlbum and the dominantScore is the one in the scoreview (e.g. the teleporting stuff, sequencer setScoreView)

 Investigate:
 - Disabled score->doLayout in ScoreView::paintEvent, did I break something???
 - Closing tab without clicking (clicking changes MuseScore::cv to the current scoreview) crashes because it thinks that it should close a tab in the second tab bar (probably fixed)

 Low Priority:
 - Albums in the start center?
 - Start center 'Open a Score...' to 'Open File...'?
 - Select elements from different scores in album-mode.
 - Score mode printing
 - There could be composer and lyricist strings in more than one place
