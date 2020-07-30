Notes:

General:
 - Album shortcuts
 - Preferences
 - Album of albums
 - Parts???
 - Undo??? Φτιαχνω για ολα τα movements κα ιγια ολα τα οργανα τα parts τους. Και μετα περνω αυτα και τα προσθετω ως movements στα parts του πρωτου
 - Single file
 - Mixer
 - Timeline

 Refactor and Code Quality:
 - Cleanup the main branch.
 - Codestyle.
 - Decouple albums and multi movement scores.
 - Add comments.
 - Refactor scoreview and mscore code.
 - Something better than cv->drawingScore()->title() == "Temporary Album Score"
 - AlbumItems from Xml should have a condition that says whether the scores have been loaded or not. If not no action should be able to happen.
 - Skip first movement in doLayout if you don't want a front cover
 - mouseREleaseEvent. Maybe update() the ScoreView???

 Bugs:
 - Fix all the places where inActiveAlbum should be inActiveAlbum and the dominantScore is the one in the scoreview (e.g. the teleporting stuff, sequencer setScoreView)

 Investigate:
 - Disabled score->doLayout in ScoreView::paintEvent, did I break something???
 - Closing tab without clicking (clicking changes MuseScore::cv to the current scoreview) crashes because it thinks that it should close a tab in the second tab bar (probably fixed)

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

 Low Priority:
 - Albums in the start center?
 - Start center 'Open a Score...' to 'Open File...'?
 - Select elements from different scores in album-mode.
 - Score mode printing
 - There could be composer and lyricist strings in more than one place
