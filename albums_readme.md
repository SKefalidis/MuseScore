Notes:
 - Improve the automated tests
 - Update .album
 - adding elements while zoomed in
 - Save in album mode?
 - Album shortcuts
 - Score mode printing
 Refactor and Code Quality:
 Bugs:
 - Crash when opening page settings in album-mode
 - Duration not updating when the duration of a score changes
 - m_tempScoreIndex goes out of sync easily
 Features:
 - page view? continuous view?
 - Add album to album
 - Albums in the start center?
 - Start center 'Open a Score...' to 'Open File...'?
 - Select elements from different scores in album-mode.


Η θεση καθοριζεται ως offset απο τον πατερα.
Ξανακαλειται το layout οποτε αλλαζει ο πατερας αλλα και το offset απο αυτον.

Αρα πρεπει να ξερουμε είτε το albumCanvasPosition
είτε τη θέση του πατέρα του album και το offset στο album´
είτε μετά από κάθε score->layout να καλείται και drawingScore->layout -> δεν δουλευει
