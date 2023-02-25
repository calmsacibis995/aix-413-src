$ @(#)28	1.3  src/tenplus/e2/common/INeditor.msg, tenplus, tenplus411, GOLD410 2/11/94 13:36:37
$
$ COMPONENT_NAME: (TENPLUS) INed Editor
$ 
$ FUNCTIONS: 
$ 
$ ORIGINS:  27
$ 
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1991, 1994
$ All Rights Reserved
$ 
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ The file that was editor.mri has been split into editor1.mri through
$ editor4.mri in order to allow it to pass through ALTU (the AIX Language
$ Translation Utility).
$
$ Error message numbers 0611-001 through 0611-150 have been allocated for
$ used in editor.mri.
$
$ TRANSLATOR:  In this file, many command (function) names are used.
$ They are all in UPPER CASE.  They should not be translated.
$ Examples:  BOX-MARK, ENTER, TAB, RIGHT-ARROW, and many more.
$ The sequences (1) (2) (3) (4) (5) (6) (7) and (8) are command names, and
$ should be left unchanged.
$
$ Most of the sets in this message catalog consist of two messages:  a short
$ message, and a long message.  The long message explains the short message.
$
$ The rest of the sets in this message catalog are menus, which consist
$ of help messages, a menu header, and menu items.
$
$ The sequence "e.g." is used in many places in this catalog.  It means
$ "for example".  It should be translated.
$
$ The sequence "i.e." is used in some places in this catalog.  It means
$ "that is" or "another way to say that is".  It should be translated.
$
$ The following phrases appear in more than one INed message catalog.  Each
$ of these phrases should be translated the same everywhere it appears:
$   1)       Editor Search Paths
$		The phrase "search path" is used to describe a list of disk
$               directory names describing locations on the disk where some
$               item of interest may be found.  The "Editor Search Paths"
$               are a collection of such lists, describing where various
$               different kinds of items of interest to the editor may be
$               found.
$   2)       Structured file
$               The INed editor can work with two kinds of files:
$               text files and "structured" files.  A text file is a normal
$               file that just contains lines of human-readable text.  A
$               structured file contains complicated combinations of data
$               and text.
$   3)       Field
$               A field is a specific place on the screen that is used to
$               display text or data that was taken from a structured file,
$               or will be put into a structured file.  Fields may be either
$               one line or many lines.  Each field has a fixed size, shape,
$               and location on the screen.
$
$quote "
$
$set MS_INEDITOR 
$
$ Translator:  do not translate anthing that looks like %s or %d or
$ %1$s or %2$d etc.
$
$ Translator: do not translate "#<number>" at the end of messages
$
M_EARGBOX "The specified command does not accept box region input.#1"
$ Users can specify regions in a file to operate on, and commands to
$ operate on the region.  One way to specify a region is as a rectangular
$ box in the file.
$ In this case, the user specified a
$ rectangular box, and then specified a command that cannot make use
$ of a rectangular box region.

H_EARGBOX "0611-001 You specified region input using either BOX-MARK or ENTER,\n\
and the positioning functions (e.g., TAB, RIGHT-ARROW). You then used\n\
a command that does not accept box region input.  Either use the\n\
command without a box region, or specify a command, such as PICK-COPY\n\
or PICK-UP, that handles box regions.\n\
\n\
Press CANCEL to continue."
$
M_EARGEMPTY "The specified command does not accept an empty parameter string.#1"
$ The user may give a null (empty) parameter to a command (function) by
$ pressing the ENTER key, and the, without typing in any parameter,
$ pressing some other command key.  Here, the user chose a command
$ that does not know what to do with such a null parameter.

H_EARGEMPTY "0611-002 You pressed ENTER, then a function.  When the sequence ENTER\n\
function is used, function receives an empty parameter string.  The\n\
function you used does not accept empty parameter strings.  Either use\n\
the function without first pressing ENTER, or press ENTER, specify a\n\
parameter that the function will accept, then use the function.\n\
\n\
Press CANCEL to continue."
$
M_EARGFILTER "Use a space between the line count and the command name in:\n\
%s#1"
$ The user can run external filter programs by pressing ENTER, and
$ then typing a command line of the form
$ count command_name DO
$ where count is a number, such as "5",
$       command_name is the name of some external filter program, and
$       DO is a command key.
$ In this case, the user either accidentally left out the space between
$ the count and the command_name, or they left out the command_name.
$ In this message, the %s will be replaced by the string the user typed.
$ Leave it on a line by itself.

H_EARGFILTER "0611-003 You tried to run a filter using ENTER Number DO without\n\
specifying a string parameter.  You either entered something like\n\
ENTER 5 DO, or ENTER 5sort DO (where there is no blank space between 5\n\
and sort).  DO accepts the following parameters:\n\
  o A positive integer (optional) (e.g., 5) to indicates the number of\n\
\tparagraphs.\n\
  o A positive integer followed by an l (e.g., 5l) to indicate the\n\
\tnumber of lines to be operated on by the filter.\n\
  o A command (e.g., sort).\n\
  o Parameters specific to the command (optional).\n\
\n\
If a positive integer is NOT provided (e.g., ENTER sort), the filter\n\
operates on the current paragraph.  If a filter has been used since\n\
you began editing, press DO to run the previously specified filter on\n\
the current paragraph.  Use ENTER DO to display a message stating\n\
which filter ran last.\n\
\n\
Press CANCEL to continue."
$ In this message, do not translate the command name "sort".  The
$ letter that appears in the phrase
$ 	l (e.g., 5l)
$ is a lower-case 'L', not the digit 1.
$
M_EARGLINE "The specified command does not accept line region input.#1"
$ The user can specify a region in a file, and then invoke a command
$ on that region.  One way to specify a region is as a set of lines
$ in the file.  In this case, the user specified such a "line region", and
$ then invoked a command that does not know how to work with a line region.

H_EARGLINE "0611-004 You specified a region input using either BOX-MARK or ENTER\n\
and the positioning functions (e.g., UP-ARROW, DOWN-ARROW).  You then\n\
used a command that does not accept region inputs.  Either use the\n\
command without a line region, or specify a command, such as PICK-COPY\n\
or PICK-UP, that handles line regions.\n\
\n\
Press CANCEL to continue."
$
M_EARGNUMBER "The specified command does not accept a numeric parameter.#1"

H_EARGNUMBER "0611-005 You used the sequence\n\
\tENTER Number Function\n\
(e.g., ENTER 10 ZOOM-IN).  This specifies a numeric parameter to the\n\
function; however, the function you specified does not accept numeric\n\
parameters.  Either use the function without using ENTER, or press\n\
ENTER, specify an parameter that the function will accept, then use\n\
the function.\n\
\n\
Press CANCEL to continue."
$
M_EARGSTRING "The specified command does not accept a string parameter.#1"

H_EARGSTRING "0611-006 You used the sequence\n\
\tENTER String Function\n\
(e.g., ENTER sort ZOOM-IN).  This specifies a string parameter to the\n\
function.  The function you used does not accept a string parameter.\n\
Either use the function without specifying ENTER, or press ENTER,\n\
specify a parameter that the function will accept, then use the\n\
function.\n\
\n\
Note that any parameter that contains both numbers and characters is\n\
considered a string parameter.  This error message appears if both\n\
numbers and characters were specified to a function that accepts only\n\
numeric parameters (e.g., ENTER 10jk FORMAT).\n\
\n\
Press CANCEL to continue."
$
M_EARGTEXT "The specified command does not accept text region input.#1"
$ The user can specify a region of a file, and then a command to operate
$ upon the specified region.  One way to specify a region is as a "text
$ region".  In this case, the user specified a text region, and then selected
$ a command that does not know how to work with a text region.

H_EARGTEXT "0611-007 You specified a region input using TEXT-MARK and the\n\
positioning functions (e.g., TAB, RIGHT-ARROW) You then used a\n\
function that does not accept text region inputs.  Either use the\n\
function without a text region, or specify a function, such as\n\
PICK-COPY or PICK-UP, that handles text regions.\n\
\n\
Press CANCEL to continue."
$
M_EBADALTFILE "Alternate file %s does not exist.#1"
$ In this message, the "%s" will be replaced with a file name.

H_EBADALTFILE "0611-008 You pressed USE to switch to the alternate file, but the\n\
alternate file no longer exists.  Before you pressed USE, the\n\
alternate file was either removed or renamed.  To switch to another\n\
file, press ENTER; then specify an existing file; then press USE.\n\
\n\
Press CANCEL to continue."
$
M_EBADDIR "File %s is not a directory.#1"
$ In this message, the "%s" will be replaced by a string that was
$ typed by the user as part of the pathname of a file.

H_EBADDIR "0611-009 You tried to edit a file, but the path to the file included a\n\
component that was not a directory.  For example, if there is a file\n\
named note with the path /usr/lisa/note and you were to type\n\
	ENTER /usr/lisa/note/memo USE\n\
the message\n\
	File /usr/lisa/note is not a directory\n\
would appear, since note is a file rather than a directory.\n\
To edit a file, specify the correct path to the file.\n\
\n\
If this error message appears while you were using MENU, some file or\n\
files have not been installed correctly.  Install INed again or use\n\
local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$ In this message, the strings "note" and "/usr/lisa/note/memo" should not
$ be translated.  The string
$ "File '/usr/lisa/note' is not a directory"
$ should be translated to match the message preceding this one.
$
M_EBADFILTER "There is a filter error on the command:\n\
%s#1"
$ In this message, the "%s" is replaced with a command line that the user
$ typed to invoke an external filter program.  Leave it on a line by itself.

H_EBADFILTER "0611-010 You tried to run a filter using ENTER Command DO.  The filter\n\
did not run, possibly because you did not type the command correctly.\n\
Use the sequence ENTER Command DO again, this time specifying an\n\
acceptable command.\n\
\n\
Press CANCEL to continue."
$
M_EBADNEXT "Do not specify a string paramter to NEXT or PREVIOUS.#1"

H_EBADNEXT "0611-011 You used ENTER String NEXT (or PREVIOUS).  The NEXT and\n\
PREVIOUS commands do not accept string parameters.\n\
\n\
Use NEXT and PREVIOUS to switch to adjacent items.  To move to a\n\
specific item in a file, you can try providing a numeric parameter to\n\
NEXT or PREVIOUS (e.g., ENTER 5 NEXT).\n\
\n\
Press CANCEL to continue."
$
M_EBADPATH "Directory %s does not exist.#1"
$ In this message, the %s will be replaced by a string that was
$ typed by the user as part of the pathname of a file.

H_EBADPATH "0611-012 You tried to edit a file, specifying a path that included a\n\
directory that does not exist.  To edit a file, specify a path that\n\
references only existing directories.\n\
\n\
If this error message appeared while you were using MENU, install INed\n\
again or use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$
M_EBOXREGION "helpbox in markregion.#1"

H_EBOXREGION "0611-013 You are defining a box or line region using the positioning\n\
functions, and either BOX-MARK or ENTER.  The ****BOX/LINE**** banner\n\
appears on the status line while you are defining box and line\n\
regions.\n\
\n\
To define a box region, position the cursor on a corner of the region,\n\
press BOX-MARK or ENTER, then position the cursor on the diagonally\n\
opposing corner of the box, or on a character in the same line.  All\n\
of the text in the region, excluding that in the rightmost column, is\n\
included in the region.  To define a line region, position the cursor\n\
on the first line of the region, press either BOX-MARK or ENTER, and\n\
mark the last line of the region using the vertical positioning\n\
functions (e.g., PAGE-DOWN, DOWN-ARROW). Once the region is defined,\n\
use INSERT-LINE, DELETE-LINE, PICK-UP, or PICK-COPY to insert space, move\n\
data, or copy data.  Use CANCEL or ENTER to stop the region definition.\n\
\n\
Press CANCEL to continue."
$ This message appears if the user hits the HELP key while in the
$ middle of either defining a box region or defining a line region.  The
$ command names BOX-MARK, ENTER, PAGE-DOWN, DOWN-ARROW, etc should not be
$ translated.  Also, the string "****BOX/LINE****" should not be translated.
$
M_EBOXSTR "****BOX/LINE****"
$
M_ETEXTSTR "******TEXT******"
$
M_EBOXTOOBIG "The region is too big to fit here.#1"

H_EBOXTOOBIG "0611-014 You tried to PUT down a box region using PUT-DOWN, PUT-COPY,\n\
or RESTORE; however, the region is too large to fit in the current\n\
field.\n\
\n\
You may have tried to PUT-DOWN multiple copies of the region by\n\
first defining the region, then using ENTER Number PUT-DOWN (or\n\
PUT-COPY or RESTORE).  This can create an area larger than the\n\
current field can handle.\n\
\n\
Press CANCEL to continue."
$
M_ECANTINSERT "Do not insert text beyond the field size limit.#1"

H_ECANTINSERT "0611-015 You tried to insert text in a field that cannot accept this\n\
text.  There is a limit to the size of this field, and you are not\n\
allowed to insert text once that size limit has been reached.\n\
\n\
Press CANCEL to continue."
$
M_ECHFONT "The available fonts are:\n\
\tr\tRoman\n\
\tw\tword underlining\n\
\tc\tcontinuous underlining\n\
\tg\tgraphics characters\n\
The current font is %s.#1"
$ In this message, the tabs (\t) are used to produce a two-column display.
$ In this message, do not translate the r, w, c, or g.
$ "Roman" is the name of the usual font.
$ The %s is replaced by one of the strings (e.g. roman, continuous underline,
$ etc) that appear in the EFONTS message set, below.

H_ECHFONT "0611-016 You used ENTER parameter FONT, where the parameter was not r,\n\
w, c, or g.  To change fonts, the parameter must be one of the\n\
following:\n\
\n\
\tr = Roman (usual font)\n\
\tw = word underlining\n\
\tc = continuous underlining\n\
\tg = graphics characters\n\
\n\
To change to one of these fonts:\n\
\tpress ENTER\n\
\ttype either r, w, c, or g\n\
\tpress FONT\n\
For example, to change to continuous underlining, you would use\n\
\tENTER c FONT\n\
\n\
Press CANCEL to continue."
$ Do not translate the separate letters r, w, c, or g.
$ The tabs (\t) are just to make the list more readable.
$
M_ECURSOR "helpbox in cursor.#1"

H_ECURSOR "0611-017 You moved the cursor to a position outside the fields on the\n\
screen.  You can edit data only when the cursor is in a field that can\n\
be edited.  To continue editing, move the cursor to a field using one\n\
of TAB, HOME, RETURN, or the arrow functions.\n\
\n\
Press CANCEL to continue."
$ When editing a directory or a structured file, the screen is broken up into
$ fields, and some places on the screen are not part of any field.  If the
$ the user moves the cursor to such a place, and then presses HELP, this
$ message is shown.
$
M_EDIRCREATE "Cannot create directory %s.#1"
$ The %s is replaced with a pathname that the user typed.

H_EDIRCREATE "0611-018 You tried to create a directory.  This operation has failed,\n\
possibly because of a lack of space.  Check directory ownership and\n\
permissions, or use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$
M_EDIRMODES "You must have execute permission in directory\n\
%s to edit the specified file.#1"

H_EDIRMODES "0611-019 You are trying to edit a file that is in a directory in which\n\
you do not have execute permission.  Change the access permissions (if\n\
possible) or ask you system administrator to do so, then try editing\n\
the file again.\n\
\n\
Press CANCEL to continue."
$
M_EDIRREADONLY "You must have directory write permission to edit the specified file.#1"

H_EDIRREADONLY "0611-020 You are trying to edit a file that resides in a directory in\n\
which you do not have write permission.  Change the permissions, if\n\
you are allowed to do so, or ask your system administrator to do so,\n\
then try again.\n\
\n\
Press CANCEL to continue."
$
M_EDISKFULL "Cannot write to file %s.\n\
Touch EXECUTE to escape to the shell and fix\n\
or touch CANCEL to exit editor without saving files.#1"
$ In this message, the %s is replaced with a file name.

H_EDISKFULL "0611-021 You tried to perform an editing function that involved\n\
writing to the disk; however, the disk is full.  You can use either\n\
EXECUTE or CANCEL:\n\
 o EXECUTE escapes the editor and brings up a shell, from which\n\
   files can be moved or deleted to create the necessary space.\n\
   Use the Ctrol-D key to quit the shell and return to the editor.\n\
 o CANCEL terminates the editing session; when CANCEL is used, some\n\
   data may be lost.\n\
\n\
Contact your system administrator if necessary.\n\
\n\
Press CANCEL to continue."
$
M_EDOBREAK "The filter was stopped by BREAK.#1"

H_EDOBREAK "0611-022 You pressed BREAK after using DO to run a filter.  Pressing\n\
BREAK stopped the editor from completing the execution of the filter,\n\
and caused this message to appear.\n\
\n\
Press CANCEL to continue."
$
M_EDOFILTER "Running filter:\n\
%s"
$ In this message, %s is replaced with the name of an external filter
$ program.
$
M_EDOTS "..."
$
M_EEMPTYNAME "The cursor must be on a file name or character string.#1"

H_EEMPTYNAME "0611-023 You used the sequence ENTER USE to switch to another file;\n\
however, the cursor is not on a file name.  To move to another file\n\
using USE, you can do one of the following:\n\
  o Press ENTER, type a file name in the popup box that appears, then\n\
    press USE or\n\
  o Move the cursor to the name of a file displayed on the screen,\n\
    then press ENTER USE.\n\
\n\
Press CANCEL to continue."
$
M_EENTER "ENTER:                             #1"

H_EENTER "0611-024 You pressed ENTER, and a popup box appeared.  ENTER is used\n\
to provide the following parameters to editor functions:\n\
 o Empty parameters (e.g., ENTER GO-TO);\n\
 o Numeric parameters (e.g., ENTER 5 INSERT-LINE);\n\
 o String parameters (e.g., ENTER sort DO); and\n\
 o BOX or LINE regions (e.g. ENTER cursor-motion PICK-UP).\n\
ENTER is also used to perform shell commands in popup boxes, using\n\
ENTER command MENU.  For example, on some systems you can use the\n\
sequence ENTER date MENU to display the date.\n\
\n\
After returning to the ENTER popup box, you can either provide\n\
a parameter to a function or to MENU, or you can CANCEL to remove\n\
the popup box.\n\
\n\
Press CANCEL to continue."
$
$ Translator:  do not translate anthing that looks like %s or %d or
$ %1$s or %2$d etc.
$
M_EFAKEMODES "Warning: Cannot fix file modes of %s.#1"
$ In this message, %s is replaced with the name of a file.

H_EFAKEMODES "0611-025 The editor has found a problem setting the file protection\n\
permissions on the current file.  Make sure the permissions are\n\
correct, use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$
M_EFBUTTON "Function key %d is not defined for this type of file.#1"
$ In this message, %d is replaced by a number.

H_EFBUTTON "0611-026 You pressed one of the functions (1) through (8).  These\n\
functions are sometimes defined to have special meanings for specific\n\
kinds of data.  However, the function you pressed has not been defined\n\
for the data you are editing.  To see if any functions are currently\n\
defined, press LOCAL-MENU.  If any of these functions are defined, you\n\
may select one of the defined functions.\n\
\n\
Press CANCEL to continue."
$
M_EFILEBAD "The editor can edit only text and structured files.#1"

H_EFILEBAD "0611-027 You are trying to edit a file that cannot be edited.  The\n\
editor can edit only text and structured files, and the file you are\n\
trying to edit is some other kind of file, such as an executable\n\
program.  Edit a file that is either text or structured.\n\
\n\
Press CANCEL to continue."
$
M_EFILEDIR "Cannot edit directory %s\n\
without both read and execute permissions.#1"

H_EFILEDIR "0611-028 You tried to edit a directory in which you do not have both\n\
read and execute permissions.  Change the permissions so that you have\n\
read and execute permissions, or ask your system administrator to do\n\
so, then try again.\n\
\n\
Press CANCEL to continue."
$
M_EFILEDOTS "Do not edit files beginning with the ... sequence.#1"
$ In this message, leave the characters ... exactly as they are.

H_EFILEDOTS "0611-029 You are trying to edit a file that cannot be edited.  The\n\
editor can edit only text and structured files, and the file you are\n\
trying to edit, whose name begins with ..., is some other kind of\n\
file.  Edit a file that is either text or structured.\n\
\n\
Press CANCEL to continue."
$ In this message, leave the characters ... exactly as they are.
$
$ This message set is a menu.  It contains a help message for the whole
$ menu, a menu header, and several items.  Each item consists of the line
$ that appears in the menu for that item, and a help message for the
$ item.  When this menu is invoked, the menu header and all the menu item
$ lines are displayed.
$
M_EFILEDRMENU "file creation menu w/ directory and re-enter#1"

H_EFILEDRMENU "0611-030 You are trying to edit a file that does not exist, so the\n\
editor has displayed a file creation menu.  You can do one of the\n\
following:\n\
 o Position the cursor on the line containing the desired option,\n\
   then press EXECUTE, or\n\
 o Press CANCEL to choose not to create a file.\n\
\n\
The options allow you to:\n\
 o Create a text file (without history).  Use for most text files.\n\
 o Create a structured file (with history).  Use for files so the\n\
   file will retain a record of changes made, or for files used\n\
   with forms having fields or multiple ZOOM levels.  History is\n\
   displayed using the File Manager.\n\
 o Create a directory.  Use to create a directory, which is used\n\
   to store other files.\n\
 o Enter the file name.  Use to specify another file.\n\
\n\
Press CANCEL to continue."
$ This is the help message for the whole menu.
$
MN_EFILEDRMENU "You are attempting to create file\n\
%s\n\
\n\
Select a menu option (for example, move the\n\
cursor to an item and press EXECUTE) or\n\
press CANCEL to remove the menu, or\n\
press HELP to display help information.\n\n\
@Create a text file (without history).\n\
@Create a structured file (with history).\n\
@Create a directory.\n\
@Enter the file name again.#4"
$
$ Do not translate '\n', '@', '#4' 
$
$ In this menu, the %s is replaced with a file name that the user has
$ typed in.
$ Keep the information about CANCEL and HELP each on it's own line.
$ IMPORTANT:  The %s must appear on the second line of the message!

H_EFILEDRMENU1 "This option creates a text file.  Text files do not keep the\n\
history information used by the File Manager.\n\
\n\
If you want to create a file that keeps history information, or a\n\
file that uses forms (i.e., has ZOOM levels or more than one field),\n\
select\n\
\tCreate a structured file (with history).\n\
\n\
CANCEL to return to the menu."

H_EFILEDRMENU2 "This option creates a structured file.  Structured files retain\n\
history information.  History is used by the File Manager to\n\
reconstruct previous versions of a structured file.\n\
\n\
Select a structured file type:\n\
 o To save the history of changes made to text in a file or\n\
 o To create files that will be used with forms having fields or\n\
   ZOOM levels.\n\
\n\
CANCEL to return to the menu."

H_EFILEDRMENU3 "Select this option to create a directory.  Directories are files\n\
used to contain other files and subdirectories.\n\
\n\
CANCEL to return to the menu."
$
$ The word "directory" here refers to standard UNIX directories.

H_EFILEDRMENU4 "Select this option to enter again the name of the file you want to\n\
edit.  If you type the name of an existing file, that file will\n\
be display.  If you type the name of a file to be created, the file\n\
creation menu will appear.\n\
\n\
CANCEL to return to the menu."
$
M_EFILEMODES "You must have read permission to edit file\n\
%s#1"
$ In this message, %s is replaced with the name of a file.  Leave it on
$ a line by itself.

H_EFILEMODES "0611-031 You tried to edit a file for which you do not have read\n\
permission.  Change the permissions so that you have read permission,\n\
if you are allowed to do so, or ask your system administrator to do\n\
so, then try again.\n\
\n\
Press CANCEL to continue."
$
$ This message set is a menu.  It contains a help message for the whole
$ menu, a menu header, and several items.  Each item consists of the line
$ that appears in the menu for that item, and a help message for the
$ item.  When this menu is invoked, the menu header and all the menu item
$ lines are displayed.
$
M_EFILERMENU "file creation menu w/o directory, w/ re-enter#1"

H_EFILERMENU "0611-032 You are trying to edit a file that does not exist, so the\n\
editor has displayed a file creation menu.  You can do one of the\n\
following:\n\
 o Position the cursor on the line containing the desired option,\n\
   then press EXECUTE or\n\
 o Press CANCEL to choose not to create a file.\n\
\n\
The options allow you to:\n\
 o Create a text file (without history).  Use for most text files.\n\
 o Create a structured file (with history).  Use for files so the\n\
   file will retain a record of changes made, or for files used\n\
   with forms having fields or multiple ZOOM levels.  History is\n\
   displayed using the File Manager.\n\
 o Enter the file name again.  Use to specify another file.\n\
\n\
Press CANCEL to continue."
$ This is the help message for the whole menu.
$
MN_EFILERMENU "You are attempting to create file\n\
%s.\n\
\n\
Select a menu option (for example, move the\n\
cursor to an item and press EXECUTE); or\n\
press CANCEL to remove the menu, or\n\
press HELP to display help information.\n\n\
@Create a text file (without history).\n\
@Create a structured file (with history).\n\
@Enter the file name again.#3"

H_EFILERMENU1 "This option creates a text file.  Text files do not keep the\n\
history information used by the File Manager.\n\
\n\
If you want to create a file that keeps history information, or a file\n\
that uses forms (i.e., has ZOOM levels or more than one field), select\n\
\tCreate a structured file (with history).\n\
\n\
CANCEL to return to the menu."

H_EFILERMENU2 "This option creates a structured file.  Structured files retain\n\
history information.  History is used by the File Manager to\n\
reconstruct previous versions of a structured file.\n\
\n\
Select a structured file type:\n\
 o To save the history of changes made to text in a file or\n\
 o To create files that will be used with forms having fields or\n\
   ZOOM levels.\n\
\n\
CANCEL to return to the menu."

H_EFILERMENU3 "Select this option to enter again the name of the file you want to\n\
edit.  If you type the name of an existing file, that file will be\n\
displayed.  If you type the name of a file to be created, the file\n\
creation menu will appear.\n\
\n\
CANCEL to return to the menu."
$
$ Keep the information about CANCEL and HELP each on it's own
$ line in the menu.
$
M_EFILESPECIAL "Cannot edit a special file.#1"
$ "Special file" here is a UNIX term for a device, such as a terminal or
$ disk drive.

H_EFILESPECIAL "0611-033 You are trying to edit a file that cannot be edited.  The\n\
editor can edit only text and structured files, and the file you are\n\
trying to edit is some other kind of file.  This file is a special\n\
file used by the system to reference devices such as terminals and\n\
disks.  Edit a file that is either text or structured.\n\
\n\
Press CANCEL to continue."
$
M_EFILEZOOM "There is no file name on current line.#1"

H_EFILEZOOM "0611-034 You pressed ZOOM-IN while on a line in the File Manager that\n\
does not contain a file name.  To ZOOM-IN to a file, move the cursor\n\
to a line containing a file name, then press ZOOM-IN.\n\
\n\
Press CANCEL to continue."
$
M_EFILTER "The current filter command line is:\n\
%s#1"
$ In this message, the %s is replaced by the name of an external filter
$ program.  Leave it on a line by itself.

H_EFILTER "0611-035 You used ENTER DO; this instructs the editor to display this\n\
message indicating the filter last used.  To run the filter shown, use\n\
DO without parameters.\n\
\n\
Press CANCEL to continue."
$
M_EFONTS "Roman\nword underline\ncontinuous underline\ngraphics"
$ This message is used to store the names of the "fonts".  These names
$ are incorporated into other messages.
$
M_EFORK "Cannot create another process now.  Try again later.#1"

H_EFORK "0611-036 The system has refused to run the program to fix the current\n\
file because the system is busy.  Try to fix the file when the system\n\
is not busy, or ask your system administrator for help.\n\
\n\
Press CANCEL to continue."
$
M_EGHOST "Trying to fix broken file %s."
$ In this message, %s is replaced with a file name.
$
M_EGNGMARGIN "Do not use margins in list fields.#1"

H_EGNGMARGIN "0611-037 You used MARGIN while in a field in a structured file where\n\
margins are not allowed.\n\
\n\
Press CANCEL to continue."
$
M_EHELPFILES "There are no HELP options specified in your editor profile.#1"

H_EHELPFILES "0611-038 Your editor profile ($HOME/profiles/editorprf) allows you to\n\
specify HELP menu options; however, no options have been specified.\n\
Possible HELP menu options are listed in\n\
/usr/lib/INed/editorprf.  Some examples are:\n\
\tAlphabetic List of Editor Commands.\n\
\tKeyboard Layouts.\n\
These options can be copied into your editorprf file HELP Options\n\
section using PICK-COPY and PUT-DOWN.\n\
\n\
Press CANCEL to continue."
$ In this message, do not translate "$HOME/profiles/editorprf"
$ or "/usr/lib/INed/editorprf"
$
$ This message set is a menu.  It contains a help message for the menu, and
$ a menu header.  It does not contain any menu items; they are stored
$ elsewhere.
$
MN_EHELPMENU "               Help Menu\n\
\n\
Select a menu option (for example, move the\n\
cursor to an item and press EXECUTE) or\n\
press CANCEL to remove the menu, or\n\
press HELP to display help information.\n#1"
$ In this menu header, center the first line above the body of text.
$ Keep the information about CANCEL and HELP each on their own line.
$ Do not translate the '\n', or '#1'

H_EHELPMENU "0611-039 You pressed HELP to display a menu of available help\n\
information.  To see help on one of the topics, move the cursor to an\n\
option, then press EXECUTE.  To exit Help and return to your file,\n\
press USE.  To remove the Help Menu without selecting an item, press\n\
CANCEL.\n\
\n\
Press CANCEL to continue."
$
M_EHGARBAGE "There is an error in the helper program.#1"

H_EHGARBAGE "The editor helper program you are using is not running properly.  The\n\
editor will try to start the editor helper program again so that you\n\
can continue editing.  If the editor helper program does not restart\n\
correctly, use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$
M_EHMGFILE "Cannot find the help message file for this editor helper program.#1"

H_EHMGFILE "0611-040 You pressed HELP while in an editor helper program, and the\n\
system has not been able to find the help message file for that editor\n\
program.  Make sure that your Editor Search Paths entry in\n\
$HOME/profiles/editorprf includes a separate line specifying\n\
\t/usr/lib/INed\n\
as a search path.  If your editor profile Editor Search Paths\n\
entry is set up correctly and this message appears, contact your\n\
system administrator.\n\
\n\
Press CANCEL to continue."
$ Do not translate /usr/lib/INed or $HOME/profiles/editorprf.
$
M_EHRESTART "There was a helper error during restart.#1"

H_EHRESTART "0611-041 The editor helper program you are using did not restart\n\
correctly, and currently is not functioning properly.  EXIT, then try\n\
using the editor again.  If the editor helper program does not work\n\
after exiting and entering the file again, use local problem reporting\n\
procedures.\n\
\n\
Press CANCEL to continue."
$
M_EIDXFILTER "Do not use the DO command here.#1"

H_EIDXFILTER "0611-042 You tried to use DO while in a structured file field where\n\
the DO command is not allowed.\n\
\n\
Press CANCEL to continue."
$
M_EIDXFORMAT "Cannot format lines in the field where the cursor is located.#1"

H_EIDXFORMAT "0611-043 You pressed FORMAT in a structured file field where the\n\
FORMAT command is not allowed.\n\
\n\
Press CANCEL to continue."
$
M_EIDXJOIN "Cannot join lines in the field where the cursor is located.#1"

H_EIDXJOIN "0611-044 You tried to combine two lines using either ENTER PICK-UP or\n\
ENTER DELETE-LINE, while in a field in a structured file where this is\n\
not allowed.\n\
\n\
To combine the lines, use PICK and PUT operations with BOX-MARK in\n\
box region mode.  Box regions are defined using BOX-MARK and the\n\
horizontal (e.g., END-LINE), and optionally the vertical (e.g.,\n\
DOWN-ARROW) positioning functions.\n\
\n\
Press CANCEL to continue."
$
M_EIDXSPLIT "Cannot split lines in the field where the cursor is located.#1"

H_EIDXSPLIT "0611-045 You tried to split a line in two using ENTER INSERT-LINE\n\
while in a structured file field where this is not allowed.\n\
\n\
To split the line, use PICK and PUT operations with BOX regions.  Box\n\
regions are defined using BOX-MARK or ENTER and the horizontal (e.g.,\n\
END-LINE), and optionally the vertical (e.g., DOWN-ARROW) positioning\n\
functions.\n\
\n\
Press CANCEL to continue."
$
M_EINDEXFILE "Cannot edit a .%s file.#1"
$ In this message, the %s will be replaced with something like "index"

H_EINDEXFILE "0611-046 You are trying to edit a file that cannot be edited.  The\n\
editor can edit only text and structured files.  The file you are\n\
trying to edit, which is either named index or concludes with the\n\
.index extension, is not a text or structured file.  Edit a file that\n\
is either text or structured.\n\
\n\
Press CANCEL to continue."
$ In this message, do not translate the words "index" or ".index".
$
M_ELINE "Line"
$ This message is used in the context of saying, for example, "line 40",
$ to indicate that you are on line number 40.  It used in the status line
$ at the bottom of the screen.
$ This message must be 4 bytes or shorter.
$
M_ELMARGIN "Do not set the left margin to the right of the right margin.#1"

H_ELMARGIN "0611-047 Your tried to set the left margin to the right of the right\n\
margin.  This is not allowed.  To set the left margin, move the cursor\n\
to a position to the left of the right margin, and press MARGIN.\n\
\n\
Press CANCEL to continue."
$
M_EMAKEDIR "Creating directory %s."
$
M_EMAKEFILE "Creating file %s."
$
M_EMINUSSEARCH "Search Up: %s"
$
M_ENEGNUMBER "Do not specify a negative numeric parameter.#1"

H_ENEGNUMBER "0611-048 You tried to provide a negative number as a parameter to a\n\
function that accepts numeric parameters.  Negative numeric parameters\n\
are not allowed.  Note, however, that negative numbers can be used as\n\
string parameters.  For example, you can use ENTER -5 SEARCH-DOWN to\n\
find the next occurrence of -5 in the field.\n\
\n\
Press CANCEL to continue."
$
M_ENEXT1 "Cannot go to the next item when not in a list.#1"

H_ENEXT1 "0611-049 You pressed NEXT while at the top level of a file.  NEXT\n\
works only when you have used ZOOM-IN while in certain fields of\n\
structured files.\n\
\n\
Press CANCEL to continue."
$
M_ENEXT2 "Cannot go to the previous item when not in a list.#1"

H_ENEXT2 "0611-050 You pressed PREVIOUS while at the top level of a file.\n\
PREVIOUS only works when you have used ZOOM-IN while in certain fields\n\
of structured files.\n\
\n\
Press CANCEL to continue."
$
M_ENEXT3 "Cannot go to the next item of this list.#1"

H_ENEXT3 "0611-051 You pressed NEXT while at a level in the file in which NEXT\n\
will not work.\n\
\n\
Press CANCEL to continue."
$
M_ENEXT4 "Cannot go to the previous item of this list.#1"

H_ENEXT4 "0611-052 You pressed PREVIOUS while at a level in the file in which\n\
PREVIOUS will not work.\n\
\n\
Press CANCEL to continue."
$
M_ENOALTFILE "There is no alternate file.#1"

H_ENOALTFILE "0611-053 You pressed USE to switch to an alternate file.  Because the\n\
file you are editing is the first file you have edited this session,\n\
there is no alternate file.  To switch to another file, use ENTER\n\
Filename USE.\n\
\n\
Press CANCEL to continue."
$
M_ENODIREDIT "Cannot edit a directory.  Edit a text or structured file.#1"

H_ENODIREDIT "0611-054 You are trying to edit a file that cannot be edited.  The\n\
editor can edit only text and structured files, and the file you are\n\
trying to edit is some other kind of file, such as a directory.  Edit\n\
a file that is either text or structured.\n\
\n\
Press CANCEL to continue."
$
M_ENOFILTER "There is no filter string set.#1"
$ A filter is a program that is separate from the editor that processes
$ text.  A filter string is a command line that includes the name of the
$ filter program, and possibly some parameters to it.

H_ENOFILTER "0611-055 You pressed DO without providing a parameter.  If a filter\n\
has been used during this editing session, using DO without a\n\
parameter will run the last filter used.  If no filter has been used,\n\
this message appears.\n\
\n\
To specify a filter, use ENTER command DO.\n\
\n\
Press CANCEL to continue."
$
M_ENOFUNCTION "The editor does not recognize the function key or keys used.#1"

H_ENOFUNCTION "0611-056 You pressed a function key that the editor does not\n\
recognize.  Select another function key.\n\
\n\
Press CANCEL to continue."
$
M_ENOHELP "There is no help available for the current error.#1"

H_ENOHELP "0611-057 No extra help information is available for the current error,\n\
question, or menu.  Use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$
M_ENONAME "[NO NAME]"
$ This message appears when a structured file is being edited, and the
$ structured file field has no name.  The phrase "NO NAME" should be
$ translated.
$
M_ENONTEXT "Use WINDOW only with text files.#1"

H_ENONTEXT "0611-058 You tried to use WINDOW while in a file that is not a text\n\
file.  An example of a non-text file is the editor profile).  WINDOW\n\
can only be used in text files.\n\
\n\
Press CANCEL to continue."
$
M_ENOSETTAB "Tabs are not allowed in the field where the cursor is located.#1"

H_ENOSETTAB "0611-059 You tried to use SET-TAB or ENTER SET-TAB to set or remove\n\
tabs in a field where tabs are not allowed.\n\
\n\
Press CANCEL to continue."
$
M_ENOSIBLING "Do not use NEXT or PREVIOUS here.#1"

H_ENOSIBLING "0611-060 You used ENTER number NEXT (or PREVIOUS) to move to a\n\
specific item; however, the data being edited is not part of a list,\n\
so NEXT and PREVIOUS do not work here.\n\
\n\
Press CANCEL to continue."
$
M_ENOSPECIALS "There is no LOCAL-MENU for this data.#1"

H_ENOSPECIALS "0611-061 You pressed LOCAL-MENU while editing data that does not have\n\
a local menu.  Since there is no local menu, this message appears.\n\
\n\
Press CANCEL to continue."
$ In this message, do translate "local menu", but do not translate
$ "LOCAL-MENU".  A "local menu" is a menu of functions that are specific to
$ the type of file the user is editing.
$
M_ENOTEXTLINE "MARGIN does not accept TEXT region input.#1"
$ The user can define a "region" in a file.  There are several different
$ types of regions.  One type is a "text" region, another is a "line"
$ region, another is a "box" region.  In this case, the user tried to define
$ a text region or a line region, and then use the MARGIN command.  The
$ MARGIN command does not work with text or line regions as inputs,
$ so this message appears.

H_ENOTEXTLINE "0611-062 You tried to use TEXT-MARK Motion MARGIN to specify both the\n\
left and right margins simultaneously.  (\"Motion\" represents the\n\
positioning functions such as the TAB and RIGHT-ARROW keys.)  MARGIN\n\
does not work with TEXT-MARK.\n\
\n\
To specify both margins at once, use BOX-MARK motion MARGIN or\n\
ENTER motion MARGIN.\n\
\n\
Press CANCEL to continue."
$ In this message, do translate the word "motion".
$
M_ENOTEXTMARK "Use a BOX region rather than a TEXT region.#1"

H_ENOTEXTMARK "0611-063 You pressed TEXT-MARK in a structured file field where this\n\
is not allowed.  To manipulate data in this kind of field, use PICK\n\
and PUT operations with BOX regions.  Box regions are defined using\n\
BOX-MARK or ENTER and the horizontal (e.g., RIGHT-ARROW), and\n\
optionally the vertical (e.g., PAGE-DOWN), positioning functions.\n\
\n\
Press CANCEL to continue."
$
M_ENOTSAVED "File %s has not been modified.#1"

H_ENOTSAVED "0611-064 You are trying to save a file, but it has not been modified,\n\
and therefore does not need to be saved.\n\
\n\
Press CANCEL to continue."
$
M_ENOWINDOWS "There are no windows to remove.#1"

H_ENOWINDOWS "0611-065 You used ENTER WINDOW when there was only one window on the\n\
screen.  ENTER WINDOW is used to remove all the windows on the screen\n\
except the one containing the cursor, and can be used only when two or\n\
more windows are displayed.\n\
\n\
Press CANCEL to continue."
$
M_ENUMTOOBIG "The numeric parameter is too large.#1"

H_ENUMTOOBIG "0611-066 You provided a numeric parameter to a function using the\n\
sequence ENTER number function.  The results of the operation are too\n\
large for the field, so the editor did not perform the operation.\n\
\n\
To perform this operation, provide a smaller numeric parameter to\n\
the function.\n\
\n\
Press CANCEL to continue."
$
M_EODDFNAME "There are characters that are not printable in the file name\n\
%s\n\
Press EXECUTE to edit the specified file or press CANCEL or HELP#1"
$ In this message, %s will be replaced with a file name.  It may
$ contain some peculiar characters.

H_EODDFNAME "0611-067 You entered a file name that contains characters that cannot\n\
be displayed (e.g., control characters).  To edit this file, press\n\
EXECUTE; otherwise, press CANCEL.\n\
\n\
Press CANCEL to continue."
$
M_EOLDHELPER "The helper program is not consistent with the editor.#1"

H_EOLDHELPER "0611-068 You are running a helper program that is not consistent with\n\
the editor; this helper may not run correctly.  Use local problem\n\
reporting procedures.\n\
\n\
Press CANCEL to continue."
$
M_EONELINE "Do not set margins in a one line field.#1"

H_EONELINE "0611-069 You tried to set margins in a one-line field.  Margins are\n\
not allowed in one-line fields.\n\
\n\
Press CANCEL to continue."
$
M_EINSERT "INSERT"
$ This string must be 9 bytes or shorter.
$ This does not refer to the INSERT-LINE key, and should not be
$ changed to INSERT-LINE.
$ Even though this is upper case, it should be translated.  It appears in
$ The status line at the bottom of the screen to indicate that the editor
$ is in insert mode (as opposed to overwrite mode).
$ If you decide that you can't translate this, you shouldn't translate the
$ next message (EOVERWRITE) either.
$
M_EOVERWRITE "OVERWRITE"
$ This string must be 9 bytes or shorter.
$ Even though this is upper case, it should be translated.  It appears in
$ The status line at the bottom of the screen to indicate that the editor
$ is in overwrite mode (as opposed to insert mode).
$ If you decide that you can't translate this, you shouldn't translate the
$ preceding message (EINSERT) either.
$
M_EPLUSSEARCH "Search Down: %s"
$
M_EPOPBIG "The following output of\n\
%s\n\
is too large for a popup box:\n"
$ In this message, the %s will be replaced by a command line (possibly with
$ parameters) that the user typed in.  Leave it on a line by itself.
$ In this message, the last line must end with \n
$
M_EPOPBREAK "The command was stopped by BREAK.#1"

H_EPOPBREAK "0611-070 You pressed BREAK while the editor was running a command in a\n\
popup box.  BREAK stopped the command and caused the editor to display\n\
this message.\n\
\n\
Press CANCEL to continue."
$
M_EPOPDONE "Touch RETURN to continue editing."
$
M_EPOPMORE "Touch RETURN to see more."
$
M_EPOPOUTPUT "There is no output from command\n\
%s#1"
$ In this message, %s will be replaced by a command line (possibly including
$ parameters) that the user typed in.

H_EPOPOUTPUT "0611-071 You tried to run a command using ENTER command MENU.  The\n\
command produced no output.\n\
\n\
If the command does not perform successfully, some characters (e.g.,\n\
sh: not found) may appear.  To erase these characters, press REFRESH.\n\
\n\
Press CANCEL to continue."
$ In the phrase "sh: not found", do not translate the "sh", but do translate
$ the "not found".  It is OK if you translate it loosely, because it
$ is just an example of what may appear.
$
M_EPREVITEM "There is no previous item in this list.#1"

H_EPREVITEM "0611-072 You pressed PREVIOUS while looking at the first item in a\n\
file; however, no items precede the one you are viewing.\n\
\n\
Press CANCEL to continue."
$
M_EPROFCOPY "Copying standard profile %s."
$ Here, %s will be replaced with the name of a file.
$
M_EPROMPT "%s                       #1"

H_EPROMPT "0611-073 You selected a menu entry that needs information.  To provide\n\
the information, type in the space provided, then press EXECUTE.\n\
Otherwise, press CANCEL.\n\
\n\
Press CANCEL to continue."
$
M_EPUTLINES "Do not put %s-type data in this field.#1"

H_EPUTLINES "0611-074 You used PICK-UP, PICK-COPY, or DELETE-LINE to get data\n\
from one type of field, then tried to use PUT or RESTORE to insert it\n\
into another type of field.  The data that you have gotten can be put\n\
only into fields having the same structure.\n\
\n\
To copy or move text from any field, use the PICK and PUT operations\n\
with BOX regions.  Box regions are defined using the BOX-MARK or ENTER\n\
commands, with horizontal (e.g., RIGHT-ARROW), and optionally the\n\
vertical (e.g., PAGE-DOWN) positioning functions.  Text PICKED using\n\
box region can be PUT into any field, regardless of the type of field.\n\
\n\
Press CANCEL to continue."
$
M_EPUTTEXT "Do not use the PUT command with a text region in a list field.#1"

H_EPUTTEXT "0611-075 You used TEXT-MARK with the positioning functions to PICK a\n\
text region, then tried to PUT the text in a data field in a\n\
structured file.  Data fields cannot accept text regions.\n\
\n\
To copy or move text from any field, use the PICK and PUT operations\n\
with BOX-MARK in box-region mode.  Box regions are defined using the\n\
horizontal (e.g., RIGHT-ARROW), and optionally the vertical (e.g.,\n\
PAGE-DOWN) positioning functions.  Text PICKED using box region can be\n\
PUT into any field, regardless of the type of field.\n\
\n\
Press CANCEL to continue."
$
M_EQUOTE "Specify an alphabetic character with the QUOTE command.#1"

H_EQUOTE "0611-076 You used QUOTE, followed by something other than 'a' through\n\
'z' or 'A' through 'Z'.  QUOTE is used to enter individual graphics\n\
characters, and it requires one of these letters as a parameter.\n\
\n\
To enter a single graphics character, use QUOTE character.\n\
\n\
Press CANCEL to continue."
$ In this message, do not attempt to translate the letters 'a' or 'z' to
$ anything appropriate for any other language.  Leave them as they are.
$
M_EREADONLY "Cannot modify this file.  Check permissions.#1"

H_EREADONLY "0611-077 You are trying to edit a file for which you do not have write\n\
permission.  Change the permissions, if you are allowed to do so, or\n\
ask your system administrator to do so, then try again.\n\
\n\
Press CANCEL to continue."
$
M_EREOPENCHANGE "Changing to %s."
$ Here, the %s will be replaced with the name of a file.  The editor is
$ switching from editing some other file to editing the file named here.
$
M_EREOPENDEL "%s has been deleted."
$ Here, %s will be replaced with the name of a file.
$
M_ERESTART_TP "Starting helper program %s again."
$
M_ERETRYING "Trying again."
$
M_ERE_ENTER_FILE "New file name:                              #1"
$ Here, leave the spaces, and translate the phrase "New file name".

H_ERE_ENTER_FILE "0611-078 Type the name of the file you wish to edit, then press\n\
EXECUTE.  Otherwise, CANCEL to remove the popup box.\n\
\n\
Press CANCEL to continue."
$
M_ERMARGIN "Cannot set the right margin to the left of the left margin.#1"

H_ERMARGIN "0611-079 You tried to set the right margin to the left of the left\n\
margin.  This is not allowed.  To set the right margin, move the\n\
cursor to a position to the right of the left margin and use ENTER\n\
MARGIN.\n\
\n\
Press CANCEL to continue."
$
M_EROFIELD "Cannot modify this field.#1"

H_EROFIELD "0611-080 This is a field that cannot be modified, or you do not have\n\
permission to modify this field.\n\
\n\
Press CANCEL to continue."
$
M_ERUNPOP "Running %s."
$ Here, %s is replaced by the name of a program, or the name of a
$ program and some parameters to the program, that is being run.
$
M_ESAVE "Cannot write to file %s\n\
(disk probably full).#1"

H_ESAVE "0611-081 You tried to save the current file under a new name; however,\n\
the editor cannot do this, possibly because the disk is full.  Contact\n\
your system administrator.\n\
\n\
Press CANCEL to continue."
$
M_ESAVE1 "Saving file %s."
$
M_ESAVE2 "Reading editor profile %s."
$ Here, the %s will be replaced with a file name.
$
M_ESAVECLOSE "Cannot close file %s.\n\
The disk may be full.  Use local problem reporting procedures.#1"

H_ESAVECLOSE "0611-082 The system has run into a problem trying to save the current\n\
file, possibly because the disk is full.  Contact your system\n\
administrator before you exit the editor.\n\
\n\
Press CANCEL to continue."
$
M_ESAVEFILE "Cannot open output file %s.#1"

H_ESAVEFILE "0611-083 You tried to edit a file, but the editor cannot open it.\n\
Either you do not have permission to create the file, you have\n\
specified a name that is not valid, or the disk is full.\n\
\n\
To edit a file, make sure the permissions allow you to create\n\
the file or directory, and make sure you have specified a legal\n\
name for the file.  If the disk is full, contact your system\n\
administrator.\n\
\n\
Press CANCEL to continue."
$
M_ESAVEFROMCOPY "Copying from %s."
$ Here, %s will be replaced with the name of a file.
$
M_ESAVETOCOPY "Copying to %s."
$ Here, %s will be replaced with the name of a file.
$
M_ESCREENDONE "\n\
Touch RETURN to continue editing."
$
M_ESHELLFAIL "Shell failed with return value = %d"
$
M_ESRCBREAK "The command was stopped by BREAK.#1"

H_ESRCBREAK "0611-084 You used SEARCH-DOWN or SEARCH-UP, then used BREAK to stop\n\
the search.\n\
\n\
Press CANCEL to continue."
$
M_ESRCFAILED "There are no occurrences of string %s.#1"
$ Here, %s will be replaced by a string that the user typed.

H_ESRCFAILED "0611-085 You used SEARCH-DOWN or SEARCH-UP, and the system did not\n\
find any occurrences of the string in the direction in which you were\n\
searching.  Make sure that you typed the search string correctly.\n\
\n\
Press CANCEL to continue."
$
M_ESRCSTRING "There is no remembered search string.#1"

H_ESRCSTRING "0611-086 You used SEARCH-DOWN or SEARCH-UP for the first time during\n\
this editing session, and you did not specify a search string.  To\n\
search for a string for the first time in an editing session, press\n\
CANCEL to remove the error message, then use ENTER String SEARCH-DOWN\n\
(or SEARCH-UP).  After a search string has been defined, you can\n\
search for the previously defined string using SEARCH-DOWN or\n\
SEARCH-UP without a parameter.\n\
\n\
Press CANCEL to continue."
$
M_ESTOBJECT "[ STRUCTURED OBJECT ]"
$
M_ETASKNAMES "popbox\nscreen\nexit\nhelper\nform\n\gly"
$
M_ETEXTREGION "helpbox in markregion#1"

H_ETEXTREGION "0611-087 You are defining a text region using TEXT-MARK and the\n\
positioning functions.  Text regions define running sequences of text\n\
by including in the region text from the initial mark to the end of\n\
the line, subsequent entire lines (if any), and text from the\n\
beginning of the line up to the cursor.  The ******TEXT****** banner\n\
appears on the status line while you define text regions.  Press\n\
CANCEL or ENTER to stop the region definition.\n\
\n\
To define a text region, position the cursor on the first character\n\
to be included in the region (e.g., the first character of a\n\
sentence), press TEXT-MARK, then position the cursor on the first\n\
character not to be included in the region (e.g., the first\n\
character in the next sentence).  Once the region is defined, use\n\
INSERT-LINE, DELETE-LINE,  PICK-UP, or PICK-COPY to insert space,\n\
move data, or copy data.\n\
\n\
Press CANCEL to continue."
$
M_ETOOMANYLINES "The specified text region is too large for the current field.#1"

H_ETOOMANYLINES "0611-088 You tried to insert a text region using PUT-DOWN, PUT-COPY,\n\
or RESTORE; however, the region is too large to fit in the current\n\
field.\n\
\n\
You may have tried to insert multiple copies of the region by first\n\
defining the region, then using ENTER Number PUT-DOWN (or PUT-COPY or\n\
RESTORE).  This can create an area larger than the current field can\n\
handle.\n\
\n\
Press CANCEL to continue."
$
M_EURESTART "Cannot start helper program %s again.#1"

H_EURESTART "0611-089 You returned to an editor helper program your were using\n\
earlier, but the editor cannot restart the editor helper program.  To\n\
continue working with the file, EXIT and enter the file again.\n\
Contact your system administrator for help, if necessary.\n\
\n\
Press CANCEL to continue."
$
M_EUSEFILE "Changing to file %s."
$
M_EUSEFORM "Cannot open form %s.#1"
$ Here, the %s will be replaced with the name of a file.

H_EUSEFORM "0611-090 You used a menu option that refers to a form that the editor\n\
cannot find.  If this message appeared when you used the Change Form\n\
menu option, then you may have specified a form name that is not\n\
valid.  Use MENU again, and provide a valid form name to the option.\n\
If this message appeared when using another menu option, install INed\n\
again or use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$ Here, a "form" is a thing that describes how text should appear on
$ the screen for the user.  The form defines which fields appear
$ where on the screen.  Forms are kept in files.  Do translate the phrase
$ "change form".
$
$ This message set is a menu.  It contains a help message for the menu, and
$ a menu header.  It does not contain any menu items; they are stored
$ elsewhere.
$
MN_EUSEMENU "             New Task Menu\n\
\n\
Select a menu option (for example, move the\n\
cursor to an item and EXECUTE).  Otherwise,\n\
press CANCEL to remove the menu, or\n\
press HELP to display help information.\n#1"
$ This is a menu header.  Center the first line over the body of text.
$ Keep the information about CANCEL on one line, and the information
$ about HELP on one line.

H_EUSEMENU "0611-091 You pressed MENU, and the editor displayed a menu of options.\n\
To select an option, use UP-ARROW and DOWN-ARROW to move to the\n\
desired option, then press EXECUTE.  Otherwise, press CANCEL to remove\n\
the New Task Menu without selecting an option.\n\
\n\
You can customize the New Task Menu by editing your editor profile\n\
$HOME/profiles/editorprf.\n\
\n\
Press CANCEL to continue."
$
M_EUSEPOP "Cannot run program %s.#1"

H_EUSEPOP "0611-092 The specified program cannot run.  Use local problem\n\
reporting procedures.\n\
\n\
Press CANCEL to continue."
$
M_EWATCHFILES "%s\n\
Press CANCEL to continue editing.#1"

H_EWATCHFILES "0611-093 A file listed in the Files the Editor Should Watch section of\n\
the $HOME/profiles/editorprf file has changed.  The editor has\n\
displayed the message associated with the file.\n\
\n\
Press CANCEL to continue."
$
$ Here, do not translate "$HOME/profiles/editorprf".  Do translate
$ "Files the Editor Should Watch"
$
M_EWATCHMSG "%s\n\
Please wait."
$ Here, the %s will be replaced by some description of what is being done
$ while the user is being asked to wait.
$
M_EWINPOS "Cannot split the window at the current cursor position.#1"

H_EWINPOS "0611-094 You tried to create a window that is too small to contain\n\
data.  To split a window vertically, place the cursor anywhere in the\n\
top line of the window except the right or left end of the top line,\n\
and press WINDOW.  To split a window horizontally, place the cursor\n\
anywhere within the window except the top or bottom line, and press\n\
WINDOW.  To remove all the windows except the one that the cursor is in,\n\
use ENTER WINDOW.\n\
\n\
Press CANCEL to continue."
$
M_EZOOMFORM "Cannot open form %s.#1"

H_EZOOMFORM "0611-095 You pressed ZOOM-IN when the cursor is in a field for which\n\
the editor can find no ZOOM form.  The ZOOM form may be installed in\n\
the wrong place, or your $HOME/profiles/editorprf Editor Search Paths\n\
may not be set up correctly.  Use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$ Do not translate "$HOME/profiles/editorprf".  Do translate "Editor
$ Search Paths".
$
M_EZOOMIN "Cannot ZOOM IN any further.#1"

H_EZOOMIN "0611-096 You pressed ZOOM-IN; however, you are not allowed to ZOOM IN\n\
any further.\n\
\n\
Press CANCEL to continue."
$
M_EZOOMOUT "Cannot ZOOM OUT any further.#1"

H_EZOOMOUT "0611-097 You tried to ZOOM-OUT in the top-level / directory; however,\n\
you are not allowed to ZOOM-OUT from /.\n\
\n\
CANCEL key to continue."
$
M_E_BAD_TERMSFILE "0611-098 The terminal definition file %s\n\
is damaged.  Install INed again or use local problem reporting procedures.\n"
$ In this message, the last line must end with \n
$
M_E_FILTERR "Filter error on command %s:#1"
$ This message will appear at the top of a popup box.  Below this
$ message, in the same box, will be whatever error message(s), if any,
$ that the filter command may have produced.

H_E_FILTERR "0611-099 You tried to run a filter using ENTER command DO.  The filter\n\
did not run, possibly because you did not type the command correctly.\n\
Use the sequence ENTER command DO again, this time specifying an\n\
acceptable command.\n\
\n\
Press CANCEL to continue."
$
M_E_LINES "0611-100 The file has too many lines.\n\
The number of lines cannot exceed %d."
$
M_E_NOROOM "The editor has run out of space.#1"

H_E_NOROOM "0611-101 You were editing a large file, and used ENTER filename USE to\n\
switch to another large file.  These files required more space than\n\
the editor has available.  The system will start the editor again on\n\
one of the files you were editing.  To edit the other file, exit from\n\
the editor, and then run the editor on the other file.\n\
\n\
Press CANCEL to continue."
$
M_E_TERM_NOT_SET "0611-102 The TERM environment variable, which names your terminal\n\
type, is not set.  Set it and export it.\n"
$ For the editor to run the "environment variable" called $ must be
$ set to be the name of the kind of terminal or display being used.
$ An environment variable is normally set from the shell command line,
$ or may be set automatically for the user when the user logs in.
$ In this message, the last line must end with \n
$
M_E_UNK_TTYTYPE "0611-103 %s is not a known terminal type.  Select one of the\n\
following:\n"
$ In this message, the last line must end with \n
$
M_E_USING_TTY "Using the %s terminal description.\n"
$ Here, %s will be replaced with the name of a type of terminal or display.
$ In this message, the last line must end with \n
$
M_E_TUNK "Terminal type %s is not recognized.\n"
$ Here, %s will be replaced with the name of a type of terminal or display.
$ In this message, the last line must end with \n
$
M_E_INSUFF_TERMINFO "%s: Insufficient terminal characteristics.\n"
$ Here, %s will be replaced with the name of a type of terminal or display.
$ In this message, the last line must end with \n
$
M_EINSERTE "IN/W_WRAP"
$ This message is used only in the Japanese version of INed; it should not
$ be translated into European languages.
$ This stands for Insert/Word_Wrap, and means that the editor is in
$ insert (as opposed to overwrite) mode,
$ and that word wrapping rules are being used.
$ This message was previously INS/ENG
$ This message must be 9 bytes or shorter.
$ This message is number 1 of a collection of 8 that should either all
$ be translated or none of them be translated.
$
M_ESINSERTE "IW"
$ This message is used only in the Japanese version of INed; it should not
$ be translated into European languages.
$ This stands for Insert/Word_Wrap, and means that the editor is in
$ insert (as opposed to overwrite) mode,
$ and that word wrapping rules are being used.
$ This message was previously IE
$ This is a short version of the preceding message.
$ This message must be 2 bytes or shorter.
$ This message is number 2 of a collection of 8 that should either all
$ be translated or none of them be translated.
$
M_EINSERTJ "IN/C_WRAP"
$ This message is used only in the Japanese version of INed; it should not
$ be translated into European languages.
$ This stands for Insert/Character_Wrap, and means that the editor is in
$ insert (as opposed to overwrite) mode,
$ and that character wrapping rules are being used.
$ This message was previously INS/JAPAN
$ This message must be 9 bytes or shorter.
$ This message is number 3 of a collection of 8 that should either all
$ be translated or none of them be translated.
$
M_ESINSERTJ "IC"
$ This message is used only in the Japanese version of INed; it should not
$ be translated into European languages.
$ This stands for Insert/Character_Wrap, and means that the editor is in
$ insert (as opposed to overwrite) mode,
$ and that character wrapping rules are being used.
$ This message was previously IJ
$ This is a short version of the preceding message.
$ This message must be 2 bytes or shorter.
$ This message is number 4 of a collection of 8 that should either all
$ be translated or none of them be translated.
$
M_EOVERWRITEE "OV/W_WRAP"
$ This message is used only in the Japanese version of INed; it should not
$ be translated into European languages.
$ This stands for Overwrite/Word_Wrap, and means that the editor is in
$ overwrite (as opposed to insert) mode,
$ and that word wrapping rules are being used.
$ This message was previously OVR/ENG
$ This message must be 9 bytes or shorter.
$ This message is number 5 of a collection of 8 that should either all
$ be translated or none of them be translated.
$
M_ESOVERWRITEE "OW"
$ This message is used only in the Japanese version of INed; it should not
$ be translated into European languages.
$ This stands for Overwrite/Word_Wrap, and means that the editor is in
$ overwrite (as opposed to insert) mode,
$ and that word wrapping rules are being used.
$ This message was previously OE
$ This is a short version of the preceding message.
$ This message must be 2 bytes or shorter.
$ This message is number 6 of a collection of 8 that should either all
$ be translated or none of them be translated.
$
M_EOVERWRITEJ "OV/C_WRAP"
$ This message is used only in the Japanese version of INed; it should not
$ be translated into European languages.
$ This stands for Overwrite/Character_Wrap, and means that the editor is in
$ overwrite (as opposed to insert) mode,
$ and that character wrapping rules are being used.
$ This message was previously OVR/JAPAN
$ This message must be 9 bytes or shorter.
$ This message is number 7 of a collection of 8 that should either all
$ be translated or none of them be translated.
$
M_ESOVERWRITEJ "OC"
$ This message is used only in the Japanese version of INed; it should not
$ be translated into European languages.
$ This stands for Overwrite/Character_Wrap, and means that the editor is in
$ overwrite (as opposed to insert) mode,
$ and that character wrapping rules are being used.
$ This message was previously OJ
$ This is a short version of the preceding message.
$ This message must be 2 bytes or shorter.
$ This message is number 8 of a collection of 8 that should either all
$ be translated or none of them be translated.
$
M_ESMALLSCREEN "CANCEL to continue.  Screen too small to display menu.#1"

H_ESMALLSCREEN "0611-104 The screen you are currently using is too small to display\n\
this menu.  Please re-size your window or display, and try again."
$
$set MS_FORMS
$
F_CMD_PROMPT "        Touch USE to continue editing, ZOOM-OUT to see list of commands.        "
$ Do not change the length of this message
$ Do not translate "USE" or "ZOOM-OUT"
$ Center the text
$
F_CMD_SUBTITLE "       Name                            Function                                 "
$ Do not change the length of this message
$
F_CMDS_TITLE "                      Alphabetical List of Editor Commands                      "
$ Do not change the length of this message
$ Center the text
$
F_CMDS_PROMPT_1 "                  Touch +LINE to see more, ZOOM-IN for detail.                  "
$ Do not change the length of this message
$ Do not translate "+LINE" or "ZOOM-IN"
$ Center the text
$
F_CMDS_PROMPT_2 "                        Touch USE to continue editing.                          "
$ Do not change the length of this message
$ Do not translate "USE"
$ Center the text
$
F_CMDS_SUBTITLE "         Name                              Function                             "
$ Do not change the length of this message
$
F_EDPRF_TITLE "                              Editor Profile File                               "
$ Do not change the length of this message
$ Center the text
$
F_EDPRF_PROMPT_1 "   This file allows you to modify the behavior of the editor to suit your       "
$ Do not change the length of this message
$
F_EDPRF_PROMPT_2 "preferences.  Put the cursor on the line of interest and ZOOM-IN to specify     "
$ Do not change the length of this message
$
F_EDPRF_PROMPT_3 "your choice of options.                                                         "
$ Do not change the length of this message
$
F_EMENU_TITLE "                                 MENU Options                                   "
$ Do not change the length of this message
$ Do not translate "MENU"
$
F_EMENU_PROMPT "                        Touch USE to continue editing.                          "
$ Do not change the length of this message
$ Do not translate "USE"
$
F_EMENU_SUBTITLE "        Description shown in menu       Type       Name of file or program      "
$ Do not change the length of this message
$
F_EMENUZ_TITLE "                             Details of MENU Option                             "
$ Do not change the length of this message
$ Do not translate "MENU"
$
F_EMENUZ_PROMPT "          Touch USE to continue editing, ZOOM-OUT to see Menu Options.          "
$ Do not change the length of this message
$ Do not translate "USE" or "ZOOM-OUT"
$
F_EMENUZ_SUBTITLE "      Description shown in menu         Type       Name of file or program      "
$ Do not change the length of this message
$
F_EMENUZ_WTEXT_1 "Flags:  any non-space character means true                                    "
$ Do not change the length of this message
$ Left justify the text
$
F_EMENUZ_WTEXT_2 "Sync and reopen file              "
$ Do not change the length of this message
$ Left justify the text
$
F_EMENUZ_WTEXT_3 "Save all files                         "
$ Do not change the length of this message
$ Left justify the text
$
F_ETASK_TITLE "                             Details of MENU Option                             "
$ Do not change the length of this message
$ Do not translate "MENU"
$ Center the text
$
F_ETASK_SUBTITLE "      Description shown in menu         Type       Name of file or program      "
$ Do not change the length of this message
$
F_ETASK_WTEXT_1 "Flags:  any non-space character means true                                    "
$ Do not change the length of this message
$ Left justify the text
$
F_ETASK_WTEXT_2 "Sync and reopen file              "
$ Do not change the length of this message
$ Left justify the text
$
F_ETASK_WTEXT_3 "Save all files                         "
$ Do not change the length of this message
$ Left justify the text
$
F_HIST_TITLE "History of file: "
$ Do not change the length of this message
$ Left justify the text
$
F_HIST_SUBTITLE "  User        Modification Time     Ins   Del   New                             "
$ Do not change the length of this message
$
F_HOWTO_PROMPT "         Touch USE to continue editing, ZOOM-OUT to see list of tasks.          "
$ Do not change the length of this message
$ Do not translate "USE" or "ZOOM-OUT"
$ Center the text
$
F_HOWTO_TITLE " Task: "
$ Do not change the length of this message
$ Center the text
$
F_HOWTO_SUBTITLE "                              How to do it                                      "
$ Do not change the length of this message
$ Center the text
$
F_HOWTOS_TITLE "                        How to Do Things With the Editor                        "
$ Do not change the length of this message
$ Center the text
$
F_HOWTOS_PROMPT_1 "   This file contains a list of things you might want to do with the editor.    "
$ Do not change the length of this message
$ Center the text
$
F_HOWTOS_PROMPT_2 "      Move the cursor to one that you are interested in and touch ZOOM-IN.      "
$ Do not change the length of this message
$ Center the text
$
F_HOWTOS_PROMPT_3 "                        Touch USE to continue editing.                          "
$ Do not change the length of this message
$ Do not translate "USE"
$ Center the text
$
F_INDEX_TITLE "                       INTERACTIVE TEN/PLUS File Manager                        "
$ Do not change the length of this message
$ Center the text
$
F_INDEX_PROMPT "            Move the cursor to an item below and ZOOM-IN to see it.             "
$ Do not change the length of this message
$ Do not translate "ZOOM-IN"
$ Center the text
$
F_INDEX_SUBTITLE "      File                               Description                            "
$ Do not change the length of this message
$
F_INDEXPRF_TITLE "                            Directory Helper Options                            "
$ Do not change the length of this message
$ Center the text
$
F_INDEXPRF_WTEXT_1 "Synchronize the index file with the directory: "
$ Do not change the length of this message
$ Left justify the text
$
F_INDEXPRF_WTEXT_2 "Directory for deleted files: "
$ Do not change the length of this message
$ Left justify the text
$
F_INDEXPRF_WTEXT_3 "Files       "
$ Do not change the length of this message
$ Left justify the text
$
F_INDEXPRF_WTEXT_4 "to be       "
$ Do not change the length of this message
$ Left justify the text
$
F_INDEXPRF_WTEXT_5 "hidden      "
$ Do not change the length of this message
$ Left justify the text
$
F_INSEQ_TITLE "   Input Sequences for:                                                         "
$ Do not change the length of this message
$
F_INSEQ_SUBTITLE "            Editor Function        Value            Key Name                    "
$ Do not change the length of this message
$
F_LS_TITLE "                       INTERACTIVE TEN/PLUS File Manager                        "
$ Do not change the length of this message
$ Center the text
$
F_LS_PROMPT "      To see detailed file status information, select an item and ZOOM-IN.      "
$ Do not change the length of this message
$ Center the text
$
F_LS_SUBTITLE "    File Name      T    Owner      Size      Modification Date     Permissions  "
$ Do not change the length of this message
$
F_MAP_TITLE "                                Keyboard Layouts                                "
$ Do not change the length of this message
$ Center the text
$
F_MAP_PROMPT_1 "                Move cursor to terminal name and touch ZOOM-IN.                 "
$ Do not change the length of this message
$ Do not translate "ZOOM-IN"
$ Center the text
$
F_MAP_PROMPT_2 "                         Touch USE to continue editing.                         "
$ Do not change the length of this message
$ Do not translate "USE"
$ Center the text
$
F_MAP_SUBTITLE_1 "                                    Terminal                                    "
$ Do not change the length of this message
$ Center the text
$
F_MAP_SUBTITLE_2 "                                      Name                                      "
$ Do not change the length of this message
$ Center the text
$
F_MAPPICT_TITLE "                   Touch ZOOM-OUT to view list of terminals.                    "
$ Do not change the length of this message
$ Do not translate "ZOOM-OUT"
$ Center the text
$
F_MAPPICT_PROMPT "                         Touch USE to continue editing.                         "
$ Do not change the length of this message
$ Do not translate "USE"
$ Center the text
$
F_OUTSEQ_TITLE "   Output Sequences for:                                                        "
$ Do not change the length of this message
$
F_OUTSEQ_SUBTITLE "     Name                       Meaning                           Value         "
$ Do not change the length of this message
$
F_PHELP_TITLE "                                  HELP Options                                  "
$ Do not change the length of this message
$ Do not translate "HELP"
$ Center the text
$
F_PHELP_SUBTITLE "      Description shown in menu         Type      Name of file or program       "
$ Do not change the length of this message
$
F_PMENU_TITLE "                                  MENU Options                                  "
$ Do not change the length of this message
$ Do not translate "MENU"
$ Center the text
$
F_PMENU_SUBTITLE "        Description shown in menu       Type       Name of file or program      "
$ Do not change the length of this message
$
F_POTHER_TITLE "                              Other Editor Options                              "
$ Do not change the length of this message
$ Center the text
$
F_POTHER_WTEXT_1 "       Wrap punctuation on FORMAT               "
$ Do not change the length of this message
$ Do not translate "FORMAT"
$ Center the text
$
F_POTHER_WTEXT_2 "       (Any non-space character means true.)    "
$ Do not change the length of this message
$ Center the text
$
F_PPATHS_TITLE "                              Editor Search Paths                               "
$ Do not change the length of this message
$ Center the text
$
F_PPATHS_PROMPT_1 "This section is used to specify the places the editor should look for forms     "
$ Do not change the length of this message
$ Left justify the text
$
F_PPATHS_PROMPT_2 "and helpers.  Each line in each box should be the name of a directory; the      "
$ Do not change the length of this message
$ Left justify the text
$
F_PPATHS_PROMPT_3 "directories are searched from top to bottom.                                    "
$ Do not change the length of this message
$ Left justify the text
$
F_PPATHS_SUBTITLE_1 "                Forms                                   Helpers                 "
$ Do not change the length of this message
$
F_PRINTPRF_TITLE "                                 Print Options                                  "
$ Do not change the length of this message
$ Center the text
$
F_PRINTPRF_PROMPT "                   ZOOM-IN to see more detail about command.                    "
$ Do not change the length of this message
$ Do not translate "ZOOM-IN"
$ Center the text
$
F_PRINTPRF_SUBTITLE_1 "      Description shown in menu                       Command                   "
$ Do not change the length of this message
$
F_PRTHELP_TITLE "                              Sample Print Options                              "
$ Do not change the length of this message
$ Center the text
$
F_PRTHELP_PROMPT_1 "                   ZOOM-IN to see more detail about command.                    "
$ Do not change the length of this message
$ Do not translate "ZOOM-IN"
$ Center the text
$
F_PRTHELP_PROMPT_2 "                         Touch USE to continue editing.                         "
$ Do not change the length of this message
$ Do not translate "USE"
$ Center the text
$
F_PRTHELP_SUBTITLE "      Description shown in menu                       Command                   "
$ Do not change the length of this message
$
F_PRTHELPZ_TITLE  "                         Sample Details of Print Option                         "
$ Do not change the length of this message
$ Center the text
$
F_PRTHELPZ_PROMPT_1  "             ZOOM-OUT to see the full list of Sample Print Options.             "
$ Do not change the length of this message
$ Do not translate "ZOOM-OUT"
$ Center the text
$
F_PRTHELPZ_PROMPT_2  "                         Touch USE to continue editing.                         "
$ Do not change the length of this message
$ Do not translate "USE"
$ Center the text
$
F_PRTHELPZ_SUBTITLE_1  "      Description shown in menu                       Command                   "
$ Do not change the length of this message
$
F_PRTHELPZ_SUB_TITLE_2  "                        Description for popbox                                  "
$ Do not change the length of this message
$
F_PRTHELPZ_WTEXT_1  "Flags:  any non-space character means true                                    " 
$ Do not change the length of this message
$ Left justify the text
$
F_PRTHELPZ_WTEXT_2  " Save all ASCII files         "
$ Do not change the length of this message
$
F_PRTHELPZ_WTEXT_3  " Print graphics box (use w/ printer filter)"
$ Do not change the length of this message
$
F_PRTHELPZ_WTEXT_4  " Clear screen and run command "
$ Do not change the length of this message
$
F_PRTHELPZ_WTEXT_5  " Display all output of command             "
$ Do not change the length of this message
$
F_PRTPRFZ_TITLE "                            Details of Print Option                             "
$ Do not change the length of this message
$ Center the text
$
F_PRTPRFZ_SUBTITLE_1 "      Description shown in menu                       Command                   "
$ Do not change the length of this message
$
F_PRTPRFZ_SUBTITLE_2 "                        Description for popbox                                  "
$ Do not change the length of this message
$
F_PRTPRFZ_WTEXT_1 "Flags:  any non-space character means true                                    "
$ Do not change the length of this message
$
F_PRTPRFZ_WTEXT_2 "Save all ASCII files          "
$ Do not change the length of this message
$ Left justify the text
$
F_PRTPRFZ_WTEXT_3 "Print graphics box (use w/ printer filter) "
$ Do not change the length of this message
$ Left justify the text
$
F_PRTPRFZ_WTEXT_4 "Clear screen and run command  "
$ Do not change the length of this message
$ Left justify the text
$
F_PRTPRFZ_WTEXT_5 " Display all output of command             "
$ Do not change the length of this message
$
F_PWATCH_TITLE "                         Files the Editor Should Watch                          "
$ Do not change the length of this message
$ Center the text
$
F_PWATCH_SUBTITLE "   Name of file                Message to display         Program to run        "
$ Do not change the length of this message
$
F_SINDEX_TITLE " Field names at current level "
$ Do not change the length of this message
$ Center the text
$
F_STAT_TITLE "                        Detailed File Status Information                        "
$ Do not change the length of this message
$ Center the text
$
F_STAT_WTEXT_1 " Full path name: "
$ Do not change the length of this message
$
F_STAT_WTEXT_2 " Description "
$ Do not change the length of this message
$
F_STAT_WTEXT_3 " File Size in Bytes: "
$ Do not change the length of this message
$
F_STAT_WTEXT_4 "  File Size in Blocks: "
$ Do not change the length of this message
$
F_STAT_WTEXT_5 " File Type: "
$ Do not change the length of this message
$
F_STAT_WTEXT_6 "  Number of Links: "
$ Do not change the length of this message
$
F_STAT_WTEXT_7 " Inode: "
$ Do not change the length of this message
$
F_STAT_WTEXT_8 "   Directory ID: "
$ Do not change the length of this message
$
F_STAT_WTEXT_9 "   Device ID: "
$ Do not change the length of this message
$
F_STAT_WTEXT_10 "   Ownership  "
$ Do not change the length of this message
$
F_STAT_WTEXT_11 " Owner of the file: "
$ Do not change the length of this message
$
F_STAT_WTEXT_12 "             Uid: "
$ Do not change the length of this message
$
F_STAT_WTEXT_13 " Group of the file: "
$ Do not change the length of this message
$
F_STAT_WTEXT_14 "             Gid: "
$ Do not change the length of this message
$
F_STAT_WTEXT_15 " Permissions  "
$ Do not change the length of this message
$
F_STAT_WTEXT_16 " Owner: "
$ Do not change the length of this message
$
F_STAT_WTEXT_17 "        Group: "
$ Do not change the length of this message
$
F_STAT_WTEXT_18 "       Other: "
$ Do not change the length of this message
$
F_STAT_WTEXT_19 " Set Uid: "
$ Do not change the length of this message
$
F_STAT_WTEXT_20 "        Set Gid: "
$ Do not change the length of this message
$
F_STAT_WTEXT_21 "       Save text image: "
$ Do not change the length of this message
$
F_STAT_WTEXT_22 "        File  "
$ Do not change the length of this message
$
F_STAT_WTEXT_23 " Time of last file access:        "
$ Do not change the length of this message
$
F_STAT_WTEXT_24 "      Access  "
$ Do not change the length of this message
$
F_STAT_WTEXT_25 " Time of last data modification:   "
$ Do not change the length of this message
$
F_STAT_WTEXT_26 "       Times  "
$ Do not change the length of this message
$
F_STAT_WTEXT_27 " Time of last file status change:  "
$ Do not change the length of this message
$
F_TINDEX_TITLE "                       INTERACTIVE TEN/PLUS File Manager                        "
$ Do not change the length of this message
$
F_TINDEX_PROMPT "            Move the cursor to an item below and ZOOM-IN to see it.             "
$ Do not change the length of this message
$ Do not translate "ZOOM-IN"
$ Center the text
$
F_TINDEX_SUBTITLE " File           Description                                                     "
$ Do not change the length of this message
$
F_TLS_TITLE "                       INTERACTIVE TEN/PLUS File Manager                        "
$ Do not change the length of this message
$
F_TLS_PROMPT "      To see detailed file status information, select an item and ZOOM-IN.      "
$ Do not change the length of this message
$
F_TLS_SUBTITLE "    File Name      T    Owner      Size      Modification Date     Permissions  "
$ Do not change the length of this message
$
F_TRM_TITLE "                      Terminal Input Sequence Descriptions                      "
$ Do not change the length of this message
$ Center the text
$
F_TRM_SUBTITLE "                                 Terminal Type                                  "
$ Do not change the length of this message
$ Center the text
$
F_TSTAT_TITLE "                        Detailed File Status Information                        "
$ Do not change the length of this message
$ Center the text
$
F_TSTAT_WTEXT_1 " Full path name: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_2 " Description  "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_3 " File Size in Bytes: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_4 "  File Size in Blocks: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_5 " File Type: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_6 "  Number of Links: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_7 " Inode: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_8 "   Directory ID: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_9 "   Device ID: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_10 "   Ownership  "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_11 " Owner of the file: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_12 "             Uid: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_13 " Group of the file: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_14 "             Gid: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_15 " Permissions  "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_16 " Owner: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_17 "        Group: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_18 "       Other: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_19 " Set Uid: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_20 "        Set Gid: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_21 "       Save text image: "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_22 "        File  "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_23 " Time of last file access:         "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_24 "      Access  "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_25 " Time of last data modification:   "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_26 "       Times  "
$ Do not change the length of this message
$
F_TSTAT_WTEXT_27 " Time of last file status change:  "
$ Do not change the length of this message
$
$set MS_PROFILES
$
P_HELP_OPT	"HELP Options"
$
P_COMM_LIST	"Display an Alphabetic List of Editor Commands."
$
P_ED_TASK	"How to Perform Various Editing Tasks."
$
P_CUSTOM	"How to Customize the Editing System."
$
P_MENU_SUGG	"Suggestions for Your MENU."
$
P_PRNT_MENU	"Suggestions for Your Print Menu."
$
P_KEY_LAYOUT	"Keyboard Layouts."
$
P_MENU_OPT	"MENU Options"
$
P_HOME_DIR	"Show Your Home Directory."
$
P_CONT		"echo \"Press Ctrl-D to continue editing\"; export PS1; PS1='eshell> '; exec sh"
$ This line is a shell command.  The punctuation, including quotes, must
$ remain exactly as it is.  Translate only "Press" and "to continue editing".
$
P_EXEC_SHELL	"Execute shell commands."
$
P_SHELL_COMM	"\\_Shell command to run:\\+"
$  TRANSLATOR:  in this message, please include the sequences \_
$  and \+ exactly as shown.
$
P_RUN_COMM	"Run an Shell Command within a Box."
$
P_SHOW_PROF	"Show Your Profiles Directory."
$
P_EDIT_PROF	"Edit Your Editor Profile."
$
P_RM_TEMP	"Remove Temporary Files."
$ Here, "housekeep" means to remove unused temporary files that the editor
$ has abandoned in your directories.
$
P_DISP_HIST	"Display the History of the Current File."
$
P_OTHER_OPT	"Other Editor Options."
$
P_SRCH_PATHS	"Editor Search Paths."
$ This refers to the set of (one or more) file path names that the editor
$ will search when trying to find certain files.
$
P_FWATCH	"Files the Editor Should Watch."
$
P_DFPRINT	"Print on the default printer."
$
P_DFPRINTING	"Printing on the default printer."
$
P_QPRT_OPT	"| qprt -T $FILE \\_qprt options:\\+"
$ Translate only the word "options", leave everything else unchanged.
$
P_PRNT_ASK	"Print (ask for options)."
$
P_PRINTING	"Printing."
$
P_PRINT_OVER	"Print to the file. Write over existing data."
$
P_PRTFILE1	"Printing to file $PRTFILE."
$ Do not tranlate PRTFILE.
$
P_PRINT_APP	"Print to the file. Append to existing data."
$
P_PRTFILE2	"Printing to file $PRTFILE."
$ Do not tranlate PRTFILE.
$
P_MAILBOX	"Mailbox."
$
P_CALENDAR	"Calendar."
