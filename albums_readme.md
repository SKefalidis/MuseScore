Notes:

General:
 - Album shortcuts (1)
 - Preferences (1)
 - Timeline (2)
 - Play panel? (1)
 - Mixer album-mode (with all the instruments)
 - Album of albums
 - Single file, export album option

 Refactor and Code Quality:
 - Cleanup the main branch.
 - Codestyle.
 - Decouple albums and multi movement scores.
 - Add comments.
 - Refactor scoreview and mscore code.
 - Something better than cv->drawingScore()->title() == "Temporary Album Score"
 - AlbumItems from Xml should have a condition that says whether the scores have been loaded or not. If not no action should be able to happen.
 - Skip first movement in doLayout if you don't want a front cover
 - MuseScore cs vs scoreView->score and drawingScore

 Parts:
 - playback
 - deleting parts

Mass changes:
 - Mass changes, Actions menu, used by both albums and groups of normal scores (multible tabs selected or opened scores)
 - Easy way to select multiple scores.
 - Actions:
    - Add - Replace instruments.
    - Change style.
    - Change footers/headers.

Investigate:
- Disabled score->doLayout in ScoreView::paintEvent, did I break something???
- Closing tab without clicking (clicking changes MuseScore::cv to the current scoreview) crashes because it thinks that it should close a tab in the second tab bar (probably fixed)

Bugs:
- Fix all the places where inActiveAlbum should be inActiveAlbum and the dominantScore is the one in the scoreview (e.g. the teleporting stuff, sequencer setScoreView)

 Low Priority:
 - Albums in the start center?
 - Start center 'Open a Score...' to 'Open File...'?
 - Select elements from different scores in album-mode.
 - Score mode printing
 - There could be composer and lyricist strings in more than one place
 - Add new movement with the same instrumentation as the last one in the album
 Split Score:
  - Easy way to split a score into an album
