$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)63	1.1  src/msg/bos/base/INhistory.msg, bosmsg, msg411, GOLD410 2/9/94 08:45:30
$
$ COMPONENT_NAME: (INED) INed Editor
$ 
$ ORIGINS:  27
$ 
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1991
$ All Rights Reserved
$ 
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$quote "
$
$set MS_INHISTORY 
$
5000 "Dummy message to add Helper message offset."
$ Translator: do not translate the above message at all
$
$ TRANSLATOR:  In this file, many command (function) names are used.
$ They are all in UPPER CASE.  They should not be translated.
$ Examples:  BOX-MARK, ENTER, TAB, RIGHT-ARROW, and many more.
$ The sequences (1) (2) (3) (4) (5) (6) (7) and (8) are command names, and
$ should be left unchanged.
$
$ Translator:  do not translate anthing that looks like %s or %d or
$ %1$s or %2$d etc.
$
$ Translator: do not translate "#<number>" at the end of messages
$
HBADFNODE "Internal Error (s_findnode failure).#1"
$ Do not translate "s_findnode failure".

H_HBADFNODE "The History Display has encountered a system problem.\n\
Contact your system administrator.\n\
\n\
Press CANCEL to continue."
$
HBADFRAME "Internal Error (frame (%1$d) >= numframes (%2$d)).#1"
$ Do not translate "frame (%1$d) >= numframes (%2$d)"

H_HBADFRAME "The History Display has encountered a system problem.\n\
Contact your system administrator.\n\
\n\
Press CANCEL to continue."
$
HBADNAME "Internal Error (mktmpnames cannot make name for temporary files.)#1"
$ Do not translate mktmpnames

H_HBADNAME "The History Display has encountered a system problem.\n\
Contact your system administrator.\n\
\n\
Press CANCEL to continue."
$
HDISKREAD "Cannot read the file %s.#1"

H_HDISKREAD "0611-361 The History Display cannot read data from the disk, possibly\n\
because of hardware problems.\n\
\n\
Press CANCEL to continue."
$
HDISKWRITE "Disk write error.#1"

H_HDISKWRITE "0611-362 The History Display cannot write data to the disk, possibly because\n\
the file system is short of space.\n\
\n\
Press CANCEL to continue."
$
HFILECHMOD "The following command has failed:\n\
chmod %1$o %2$s#1"
$ Do not translate chmod.

H_HFILECHMOD "0611-363 The History Display has encountered a system problem.\n\
Contact your system administrator.\n\
\n\
Press CANCEL to continue."
$
HFILECREATE "Cannot create the file %s.#1"

H_HFILECREATE "0611-364 The specified file name is not valid.\n\
\n\
Press CANCEL to continue."
$
HFILENAME "Enter file name (%s):                            #1"
$
$ In this message, leave the trailing spaces. The exact length of the
$ spaces is not critical, but make it about this long.

H_HFILENAME "This popup box requests the name to be assigned to the file version\n\
to be saved.  The default name, listed in parentheses, is the name of\n\
the file you are editing.  To accept the default name, press EXECUTE.\n\
This saves this version of the file under the default name, and backs\n\
up the version of the file you were editing.  If you do not choose to\n\
accept the default name, type the new file name, then press EXECUTE.\n\
If you do not want to save this version of the file, press CANCEL.\n\
\n\
Press CANCEL to continue."
$
HFILEOPEN "Cannot open file %s.#1"

H_HFILEOPEN "0611-365 The History Display cannot write data to the disk, possibly\n\
because the file system is short of space.\n\
\n\
Press CANCEL to continue."
$
HFILEWRITE "Cannot write to file %s.#1"

H_HFILEWRITE "0611-366 The History Display cannot write data to the disk, possibly\n\
because the file system is short of space.\n\
Delete some files to make some disk space available.\n\
\n\
Press CANCEL to continue."
$
HHISTERR "Token type %d is not valid or not expected.#1"

H_HHISTERR "0611-367 There is a problem with the file history.\n\
Contact your system administrator.\n\
\n\
Press CANCEL to continue."
$
HLOOKFIRST "You are looking at the first version of the file."
$
HLOOKLAST "You are looking at the last version of the file."
$
HMAKING "Making the history file."
$
HMENU "u9 menu#1"

H_HMENU "Use the History Display to view and restore previous versions of the\n\
file being edited.  The History of File screen lists versions of the\n\
file, arranged by editing date.  To view a previous version of the file,\n\
position the cursor on a date then press ZOOM-IN. To return to the\n\
History of File screen, press ZOOM-OUT.  To return to the current version\n\
of the file, press ZOOM-OUT while viewing the History of File screen.\n\
\n\
The History Display Local Menu options are:\n\
 (1) Show Time of this Version of the File - Shows version date.\n\
 (2) Show Next Time - Displays the next version of the file.\n\
 (3) Show Previous Time - Displays the previous version of the file.\n\
 (4) Redisplay History - Displays the History of File screen.\n\
 (5) Save Current Version of File - Saves the displayed version of\n\
     the file using a file name you specify.\n\
\n\
Press CANCEL to continue."
$
MN_HMENU "\t       History Display\n\
\n\
Select a menu option by moving the cursor to\n\
an item and pressing EXECUTE.  Press CANCEL\n\
to remove the menu, or press HELP to display\n\
help information.\n\n\
@(1) Show Time of this Version of the File\n\
@(2) Show Next Time\n\
@(3) Show Previous Time\n\
@(4) Redisplay History\n\
@(5) Save Current Version of File#5"
$
$ Do not translate the '@', '\n', '\t', or #5
$ This is a menu header.  Keep the lines about 40 to 50 characters long,
$ and center the first line above the text body.

H_HMENU1 "Use Local Menu option\n\
\t(1) Show Time of This Version of the File\n\
to determine the date of the version you are viewing.  Select (1) after\n\
you press ZOOM-IN from the History of File screen.\n\
\n\
To save the displayed version, select option\n\
\t(5) Save Current Version of File.\n\
To return to the file you were editing press ZOOM-OUT to go to the\n\
History of File screen, or select option \n\
\t(4) Redisplay History.\n\
Then press ZOOM-OUT to return to the file you were editing.\n\
\n\
Press CANCEL to continue."

H_HMENU2 "This option displays the next subsequent version of the file.\n\
Select (2) after you press ZOOM-IN from the History of File screen.\n\
\n\
See the previous version of the file by selecting option\n\
\t(3) Show Previous Time.\n\
To return to the file you were editing press ZOOM-OUT to go to the\n\
History of File screen, or select option \n\
\t(4) Redisplay History.\n\
Then press ZOOM-OUT to return to the file you were editing.\n\
\n\
Press CANCEL to continue."

H_HMENU3 "This option displays the most recent version of the file.\n\
Select option (3) after using ZOOM-IN from the History of File screen.\n\
\n\
To see the next version of the file, select option\n\
\t(3) Show Next Time.\n\
To return to the file you were editing press ZOOM-OUT to go to the\n\
History of File screen, or select option \n\
\t(4) Redisplay History.\n\
Then press ZOOM-OUT to return to the file you were editing.\n\
\n\
Press CANCEL to continue."

H_HMENU4 "This option returns to the History of File screen.\n\
Select option (4) after using ZOOM-IN from the History of File screen.\n\
\n\
To return to the file being edited, press ZOOM-OUT while viewing the\n\
History of File screen.\n\
\n\
Press CANCEL to continue."

H_HMENU5 "This option saves the version of the file being viewed.\n\
Select option (5) after using ZOOM-IN from the History of File screen.\n\
\n\
A popup box appears and requests the name to be assigned to this\n\
version of the file.  The default name, listed in parenthesis, is\n\
the name of the file you are editing.  To accept the default name,\n\
press EXECUTE.  This saves the version of the file being viewed\n\
under the default name, and backs up the version of the file you were\n\
editing.  If you do not choose to accept the default name, type the\n\
new file name, then press EXECUTE.  If you do not want to save this\n\
version of the file, press CANCEL.\n\
\n\
Press CANCEL to continue."
$
HMODE "HISTORY-MODE"
$
HNOHIST "File %s does not have any history.#1"

H_HNOHIST "0611-368 The file being edited has no history. To save the history of\n\
this file, convert the file to a structured file with the newfile command.\n\
\n\
Press CANCEL to continue."
$ Do not translate newfile.
$
HNOMENU "There is no LOCAL-MENU for this data.#1"

H_HNOMENU "0611-369 You used LOCAL-MENU while editing data that does not have a\n\
Local Menu.\n\
\n\
Press CANCEL to continue."
$
HNOSAVE "Cannot save the current version in file %s.#1"

H_HNOSAVE "You tried to save the current file version in a file already used by the\n\
History Display.  This is not allowed.  Select Local Menu option\n\
\t(5) Save Current Version of File\n\
then specify another name for this version of the file.\n\
\n\
Press CANCEL to continue."
$
HREGION "This helper does not understand regions.#1"

H_HREGION "You tried to use TEXT-MARK or BOX-MARK while in the History Display.\n\
This is not allowed.\n\
\n\
Press CANCEL to continue."
$
HSAVE "I/O Error (The disk is probably full.)#1"

H_HSAVE "The History Display cannot save your file, probably because the\n\
file system lacks space.  Delete some files to release disk space.\n\
\n\
Press CANCEL to continue."
$
HSAVING "Saving this version as file %s."
$
HU1 "You are not looking at a previous version of the file.#1"

H_HU1 "You used Local Menu option\n\
(1) Show Time of this Version of the File\n\
which displays the date of the version of the file being viewed.\n\
However, you are looking at the History of File screen, rather than\n\
at a previous version of the file. To view a previous version of a file,\n\
move the cursor to one of the dates in the History of File screen, then\n\
press ZOOM-IN.\n\
\n\
Press CANCEL to continue."
$
HU2 "You are not looking at a previous version of the file.#1"

H_HU2 "You used Local Menu option\n\
\t(2) Show Next Time\n\
which displays the next version of the file.  However, you are looking\n\
at the History of File screen, rather than at a previous version of the\n\
file. To view a previous version of a file, move the cursor to one of the\n\
dates in the History of File screen, then press ZOOM-IN.\n\
\n\
Press CANCEL to continue."
$
HU3 "You are not looking at a previous version of the file#1"

H_HU3 "You used Local Menu option\n\
\t(3) Show Previous Time\n\
which displays the previous version of the file.  However, you are looking\n\
at the History of File screen, rather than at a previous version of the\n\
file. To view a previous version of a file, move the cursor to one of the\n\
dates in the History of File screen, then press ZOOM-IN.\n\
\n\
Press CANCEL to continue."
$
HU5 "You are not looking at a previous version of the file.#1"

H_HU5 "You used Local Menu option\n\
\t(5) Save Current Version of File\n\
which saves a previous version of the file.  However, you are looking\n\
at the History of File screen, rather than at a previous version of the\n\
file. To view a previous version of a file, move the cursor to one of the\n\
dates in the History of File screen, then press ZOOM-IN.\n\
\n\
Press CANCEL to continue."
$
HUINIT "uinit: Cannot get status on file %s.#1"

H_HUINIT "The History Display found a system problem while trying to\n\
look at the file history.\n\
\n\
Press CANCEL to continue."
$
HVERSION "You are looking at the version made on %s."
$ Here, the %s will be replaced with a time/date string.
$
HZOOMIN "The cursor must be on a line that is not blank.#1"

H_HZOOMIN "You pressed ZOOM-IN while the cursor was on a blank line in the\n\
History of File screen.  This is not allowed.  Position the cursor on\n\
a line containing a date, then press ZOOM-IN to display the version of\n\
the file that existed on that date.\n\
\n\
Press CANCEL to continue."
$
HVIEWING "You are viewing the history of a file already.#1"

H_HVIEWING "You pressed ZOOM-IN from the History of File screen.  You then tried to\n\
invoke the History Display.  This is not allowed, since you are viewing\n\
the history of a file already.\n\
\n\
Press CANCEL to continue."
