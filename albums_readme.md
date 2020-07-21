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
 - Closing tab without clicking (clicking changes MuseScore::cv to the current scoreview) crashes because it thinks that it should close a tab in the second tab bar (probably fixed)
 - Contents not generated when the first score does not have a lyricist?
 - Contents correct names!!!
 - Add instrument to the active score (very buggy)
 - Crash when adding clicking instrument name in album mode
 - Fix all the places where inActiveAlbum should be inActiveAlbum and the dominantScore is the one in the scoreview (e.g. the teleporting stuff)
 Features:
 - Albums in the start center?
 - Start center 'Open a Score...' to 'Open File...'?
 - Select elements from different scores in album-mode.
