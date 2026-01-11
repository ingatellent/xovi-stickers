# reMarkable stickers for xovi

This is a xovi extension which adds the following functionality

- save the clipboard to a file
- read a saved clipboard from file and convert to scene items
- change the color, pen or thickness of scene items (e.g. from the clipboard)

In order to handle saved clipboards, to be used as stickers, it also contains functionality to
- ensure that a certain directory exists
- delete a file (use carefully!)

The repository (and the built binary) also contains a qmd file which adds a stickers button to the toolbar, but developers can also make their own.

Currently the methods for changing thickness of strokes does not change the bounding box, so thicker strokes will overflow the box (and be cut off).

The repo also contains a qmd (addEditsToSelection.qmd) with examples on how to use the methods for changing style of scene items. This is not state of the art and is very much work in progress. It adds 6 buttons to the selection context menu, two for changing pen, two for size and two for colors.

It's not so difficult to also render strokes from .svg files, but one needs to parse the svg (or find a c++ svg parser to include).

PRs or suggestions are welcome, but any PR with unhashed qmd files will be rejected.

The clipboard interaction as well as all work regarding rm_Line, rm_SceneItem and rm_SceneLineItem was originally created by HookedBehemoth <https://github.com/HookedBehemoth/xovi-sudoku>. This work was released under GPL v2, and hence that is also the license of this repository.

