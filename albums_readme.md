Notes:
 - Improve the automated tests
 - Album shortcuts
 - Score mode printing
 - Active album, Active score behavior
 - Album of albums
 - Continuous View (disable temporarily), need a workaround, maybe score made with every score in continuous view?

 Refactor and Code Quality:
 - Cleanup the main branch

 Bugs:
 - Contents correct names!!!
 - Add instrument to the active score (very buggy)
 - Fix all the places where inActiveAlbum should be inActiveAlbum and the dominantScore is the one in the scoreview (e.g. the teleporting stuff)

 Investigate:
 - Disabled score->doLayout in ScoreView::paintEvent, did I break something???
 - Closing tab without clicking (clicking changes MuseScore::cv to the current scoreview) crashes because it thinks that it should close a tab in the second tab bar (probably fixed)

 Features:
 - Albums in the start center?
 - Start center 'Open a Score...' to 'Open File...'?
 - Select elements from different scores in album-mode.
