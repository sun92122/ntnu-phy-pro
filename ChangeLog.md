# SciDAVis changelog (main changes)

=== Release 2.1 ===
- Qt4 support was dropped

=== Release 1.26 ===
- Added simplified and traditional Chinese translations, courtesy of Anbang Li and Wen Li
- Updated Czech translation from Pavel Fric
- some bugs were fixed

=== Release 1.25 ===
- added support for Qt 5.x;
- liborigin updated;
- some bugs were fixed;
- color picker for symbol edge and fill colors was enhanced
- Color settings are saved as hex values in .sciprj files

=== Release 1.23 - 2018-06-04 ===
- Bug fix release;
- added Python 3 support
- dropped Qt3 support
- Russian translation updated
- Origin import support updated

# The events below are more for historical reference than for recording changes in SciDAVis

# Changelog between versions 1.D1 and 1.22

=== Release 1.22 - 2017-10-22 ===
- Bug fix release;
- add calculation and display of curve integral

=== Release 1.21 - 2017-08-08 ===
- Bug fix release

=== Release 1.18 - 2017-06-21 ===
- Bug fix release

=== Release 1.17 - 2017-05-31 ===
- Bug fix release

=== Release 1.14 - 2016-07-28 ===
- Bug fix release;
- new numbering format

=== Release 1.D13 - 2016-06-05 ===
- Bug fix release

=== Release 1.D9 - 2015-11-24 ===
- Bug fix release

=== Release 1.D8 - 2014-07-23 ===
- Bug fix release

=== Release 1.D5 - 2014-02-15 ===
- Bug fix release

=== Release 1.D4 - 2014-01-25 ===
- Bug fix release

=== Release 1.D1 - 2013-12-27 ===
- Bug fix release;
- New project leader;
- new numbering format;
- version bump

# Changelog before SciDAVis 1.D1

=== Release 0.2.4, 2010-03-12 ===
Most important changes (since 0.2.3)
- Some parts of the Python API have been marked as deprecated and generate warnings when used. This
  may be a bit unusual for a bugfix release; on the other hand, they won't be removed any time soon
  and inserting the deprecation warnings now will give everyone as much time as possible for getting
  accustomed to the API we're moving towards.
  Most prominently,
    Table.cell(column,row) and
	 Table.setCell(column,row,value)
  are deprecated in favour of
    Table.column(column - 1).valueAt(row - 1) and
    Table.column(column - 1).setValueAt(row - 1, value)
  (note that the column/row INDICES DIFFER BY ONE!); the same goes for
    Table.text(c,r) -> Table.column(c - 1).textAt(r - 1) and
    Table.setText(c,r,text) -> Table.column(c - 1).setTextAt(r - 1,text).
  This serves multiple purposes. It simplifies the interfaces by grouping all functionality specific
  to a certain column in the Column class; it allows column objects to be stored and passed around;
  and it moves from the unconventional 1-based indexing to the more conventional 0-based indexing.
  Future additions to the API will increasingly make use of the columns-as-objects paradigm.
- lots of bugfixes, again including crashes
- fixed compatibility issues with Qt 4.6 and SIP 4.9
- performance improvements
- substantially improved support for plots involving date/time data
- added two Czech translations: a default one by Pavel Fric (feedback requested via
  http://fripohled.blogspot.com) and an alternative one by Jan Helebrant
- updated Spanish translation, thanks to Mauricio Troviano; Brazilian Portuguese translation, thanks
  to Fellyp do Nascimento; and German translation
- fixed and extended many parts of the API for Python scripts, including the new methods
    Layer.pickPoint()
	 MDIWindow.clone()
	 Folder.save(filename)
	 newGraph(string)
	 Layer.printDialog()
	 Graph.printDialog()
	 Layer.setRightTitle(string)
	 Layer.setTopTitle(string)
	 Layer.grid() # => returns an instance of the new class Grid
	 Layer.insertFunctionCurve(formula,from=0,to=1,points=100,title="")
	 Layer.insertPolarCurve(radial_formula,angular_formula,from=0,to=2*pi,parameter="t",points=100,title="")
	 Layer.insertParametricCurve(x_formal,y_formula,from=0,to=1,parameter="t",points=100,title="")
	 Matrix.recalculate()
  the constants QwtPlot.yLeft, QwtPlot.yRight, QwtPlot.xBottom, QwtPlot.xTop for specifying axes
  and the Grid class
- multiple graphs can have active tools simultaneously
- based on user feedback, the default behaviour when importing ASCII files has been changed to
  interpret the data as numeric (only applies to fresh installations of SciDAVis)
- automatic resizing of table rows for multi-line cells intentionally dropped, because this seems
to be rarely needed and has a huge impact on performance for large tables

=== Release 0.2.3, 2009-07-05 ===
Most important changes (since 0.2.2)
- fixed many more bugs, including various crashes
- performance improvements
- improved pasting of cells from OpenOffice.org Calc and KSpread into tables
- Brazilian Portuguese and German translations improved
- Some build system fixes for building on MacOS X Tiger and detecting lupdate-qt4/lupdate
- now also compatible with Qwt 5.2.0
- main window remembers size from the last session instead of always displaying maximized
- added the following methods to the Python interface:
    MDIWindow.name()
	 MDIWindow.setName(string)
	 Note.text()
	 Note.setText(string)
	 Note.executeAll()
	 Note.setAutoexec(bool)
	 Note.exportASCII(filename)
	 Note.importASCII(filename)

=== Release 0.2.2, 2009-04-19 ===
Most important changes (since 0.2.1)
- added Brazilian Portuguese translation by Fellype do Nascimento
- fixed opening of legacy files with non-numeric data
- fixed various crashes
- fixed importing of numeric data as new rows or overwriting the current table
- fixed various issues with invalid/empty cells
- implemented a more sophisticated file-saving protocol, which is supposed to
  protect against data loss in the event of a system crash shortly after saving
  the file; see
  http://bugs.launchpad.net/ubuntu/+source/linux/+bug/317781/comments/54
- performance improvements
- Python function Table.importASCII() reimplemented, which was missing in
  SciDAVis 0.2.0 and 0.2.1.

=== Release 0.2.1, 2009-03-09 ===
Most important changes (since 0.2.0)
- fixed saving of project files with function/fit curves
- fixed opening of backup copy on discovery of a corrupt project file
- Spanish translation update by Mauricio Troviano
- fixed several regressions introduced by the table/matrix rewrites in 0.2.0
- added per mille and per ten thousand signs to SymbolDialog
- compatibility problems between SIP versions triggers a warning instead of a
  crash (happens sometimes when SIP version at runtime differs from the one
  used at compile time)

=== Release 0.2.0, 2009-02-14 ===
Most important changes (since 0.1.4)
- multi-level undo/redo for all operations on
  tables and matrices
- many operations on tables and matrices now support
  non-contiguous selections
- the important options/controls for matrices
  and tables are now integrated in a sidebar
  of control tabs which make working with
  column or matrix based data much more
  convenient (almost no more opening and closing of
  dialogs necessary)
- tables now support different formulas
  for each cell
- numeric values are now stored independent of
  their textual representation, i.e., you don't
  lose data when hiding decimal digits
- formula edit mode: tables feature a new
  mode which allows the user to individually edit
  the formula for each cell 
- invalid cells: tables now mark cells without
  input as invalid rather than treating them
  as 0; these cells are ignored in plots
- date/time values are now internally stored
  as QDateTime objects which opens up many
  new possibilities of date/time manipulation 
  using PyQt/Python scripting
- many bug fixes

=== Release 0.1.4, 2009-02-08 ===
Most important changes (since 0.1.3)

- Weighted linear and polynomial fits required actual weights
 (1/sigma^2) to be specified in place of standard errors, while all others
 (as well as error bar plots) needed the standard errors themselves.
 Solution:  Switched everything to using standard errors, eliminating the term
				"weighting" from the GUI and most of the source code.
 Rationale: Avoid confusion for users, especially given the previous
				inconsistent treatment of the matter. Avoid confusion for
				devs. Arbitrary weights don't work naturally with non-linear fits;
				and they almost certainly lead to nonsensical error estimates of
				results (unless you _really_ know what you're doing, in which case
				you can simply specify 1/sqrt(weight) as standard error).

- Fixed estimation of parameter errors in case of non-linear fitting with
  unknown Y errors.

- Fixed computation of R^2 in case of known Y errors.

- Fixed handling of parameters in fit curve formulas as well as constant
  fit parameters; both of which used naive string-substitution; i.e. if
  you used parameter names which appeared elsewhere as substrings of other
  parameter names or functions, you'd get weird errors. Unfortunately, the fix
  makes non-linear fitting with user-supplied formulas a bit slower; hopefully,
  we'll come up with a Really Good Solution(tm) for one of the next major
  releases.

- Fixed problem with FFT filters which could lead to incorrect results:
  Cutoffs could fall between real and imaginary part of a Fourier component,
  producing a reduced amplitude and bogus phase at this point.

- Fixed crash when creating bargraph with errorbars.

- New Russian translation by f0ma.

- Ported the hack that prevents axes gaps in graph vector exports
  (eps, pdf, svg) from QtiPlot.

- Numerous small tweaks and bug fixes.

=== Release 0.2.0-beta1, 2008-08-10 ===
Most important changes (since 0.1.3)
- multi-level undo/redo for all operations on
  tables and matrices
- many operations on tables and matrices now support
  non-contiguous selections
- the important options/controls for matrices
  and tables are now integrated in a sidebar
  of control tabs which make working with
  column or matrix based data much more
  convenient (almost no more opening and closing of
  dialogs necessary)
- tables now support different formulas
  for each cell
- numeric values are now stored independent of
  their textual representation, i.e., you don't
  lose data when hiding decimal digits
- date/time values are now internally stored
  as QDateTime objects which opens up many
  new possibilities of date/time manipulation 
  using PyQt/Python scripting

Known issues (to be fixed until 0.2.0 stable):
- the Python bindings for the new table
  features are not finished yet
- there is currently no way to display which
  formula has been assigned to which cell
  in a table
- the date/time column functionality has
  not been completely integrated and may
  lead to empty plots in many cases

=== Release 0.1.3, 2008-04-19 ===
Most important changes (since 0.1.2)
- page orientation for PDF/EPS export can now be chosen manually
- executing script code now gives a visual feedback
- new Windows installer
- many improvements to make compilation and Linux packaging easier
- fixed graph duplication
- fixed unnecessary graph rescaling
- several other bugs fixed

=== Release 0.1.2, 2008-02-03 ===
Most important changes (since 0.1.1)
- fixed crash when changing the type of a curve
- fixed crash when plotting an empty table
- fixed the fit wizard which did not accept function definition
- fixed pasting into tables
- fixed some other minor issues

=== Release 0.1.1, 2007-12-21 ===
Most important changes (since 0.1.0)
- several bugs fixed

=== Release 0.1.0, 2007-08-05 ===
Most important changes (since QtiPlot 0.9-rc2):
- improved handling of custom decimal separators
- many improvements/additions to Python scripting
- improved user interface: restructured menus and tool bars, several restructured dialogs
- script window removed since notes offer the same capabilities
- more stable project loading/saving
- easier ASCII import/export, improved appending of projects and Origin graphs
- lots of small improvements and bug fixes
- export to eps and pdf without large white borders
