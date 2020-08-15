Notes:

Today:
    - improve multi-movement playback in seq (I want section break pauses in this)
    - seq pause, for multi-movement correct behaviour
    - bug with cursor and parts. setActiveScore is a called a bunch of times and things get confusing and it either chooses
    an excerpt or the main score for the cursor

Tests:
    - compressed album file Test (almost done)
    - old .album files

Week 2 - 3:
    Bugfixes and refactoring.
    - Check all TODOs
    - Add the new members of MasterScore/Score to clone functions and save/write?
    Git cleanup.
    Preferences.
    Contents page -> pages + improved alignment.
    Decide if I allow only one active album or not. If not rename, partOfActiveAlbum to partOfAlbum or something.
    - Improve the draw/don't draw front cover option
    - Separate .album from .msca/.mscaz in the open options

Week 4:
    Shortcuts.
    Select elements from different scores in album-mode.
    Mass changes:
     - Mass changes, Actions menu, used by both albums and groups of normal scores (multible tabs selected or opened scores)
     - Easy way to select multiple scores.
     - Actions:
        - Add - Replace instruments.
        - Change style.
        - Change footers/headers.

Week 5+ (further improvements):
    Mixer for album-mode (with all the instruments).
    Parts with different instrumentation.
    Timeline for the entire Temporary Album Score.
    Albums inside an Album.
    Tools to split a score into multiple scores (an album).
    Improved composer/lyricist handling. Give the user an option in the inspector to include (or not) that string in the front cover.
    Add new movement with the same instrumentation as the last one in the album


 Refactor and Code Quality:
 - Decouple albums and multi movement scores.
 - Refactor scoreview and mscore code.
 - Something better than cv->drawingScore()->title() == "Temporary Album Score"
 - AlbumItems from Xml should have a condition that says whether the scores have been loaded or not. If not no action should be able to happen.
 - Skip first movement in doLayout if you don't want a front cover
 - MuseScore cs vs scoreView->score and drawingScore
 - function that checks if the parts of the album are incompatible (+ we should not check the dominant score for incompatible parts, it does not have any measures)
 - function that deletes all parts from the album

Investigate:
 - Disabled score->doLayout in ScoreView::paintEvent, did I break something???
 - Closing tab without clicking (clicking changes MuseScore::cv to the current scoreview) crashes because it thinks that it should close a tab in the second tab bar (probably fixed)

Bugs:
 - Fix all the places where inActiveAlbum should be inActiveAlbum and the dominantScore is the one in the scoreview (e.g. the teleporting stuff, sequencer setScoreView)
 - Crash when editing title of part
 - james' list
 - crashes related to text (are this all on master?)
 - crash when changing to part and dragging the view (without doing anything before that)
 - restore session crash in paintEvent (does not crash if you select an element first)
