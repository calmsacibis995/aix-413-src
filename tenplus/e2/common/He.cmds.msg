$ $*@(#)32      1.2  src/tenplus/e2/common/He.cmds.msg, tenplus, tenplus411, GOLD410 7/26/93 15:49:56

$ $*COMPONENT_NAME: (INED) INed Editor
$
$ $*ORIGINS:  27
$
$ $*This module contains IBM CONFIDENTIAL code. -- (IBM
$ $*Confidential Restricted when combined with the aggregated
$ $*modules for this product)                  SOURCE MATERIALS
$ $*(C) COPYRIGHT International Business Machines Corp. 1991
$ $*All Rights Reserved

$ $*US Government Users Restricted Rights - Use, duplication or
$ $*disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

$ TRANSLATOR:  In this file, many command (function) names are used.
$ They are all in UPPER CASE.  They should not be translated.
$ Examples:  BOX-MARK, ENTER, TAB, RIGHT-ARROW, and many more.
$ The sequences (1) (2) (3) (4) (5) (6) (7) and (8) are command names, and
$ should be left unchanged.
$ A "popup box" is a small box that appears on the screen on top of
$ whatever was there before, and contains a short message.  When the popup
$ box is removed from the screen, the former contents of the screen reappear.
$ The word "pick" is used to describe picking up text from the file that is
$ being edited (the text disappears from the file), and storing it in a
$ temporary buffer.
$ The word "put" is used to describe the opposite action -- inserting the
$ text into the file being edited.

$quote "
$len
$set MS_HECMDS

FUNCTION_KEYS1	"Menu functions (1) to (8)."
FUNCTION_KEYS2	"Functions (1) to (8) perform operations specific to certain kinds of data,\n\
or invoke options when any menu appears.  Press LOCAL-MENU to receive a\n\
menu of these functions.  You can select either the Local Menu option from\n\
the menu, or use (1) - (8) directly.  You do not have to display the menu.\n\
Each function key corresponds to a line on the menu.  You can also invoke\n\
these options when viewing any menu; normally, the first option can be\n\
invoked using (1), the second invoked using (2), and so on.  In certain\n\
situations, you can provide parameters to these functions, using the\n\
following sequence:\n\
  o Press ENTER\n\
  o Enter the desired parameter\n\
  o Press the appropriate function key.\n\
\n\
To get more information on Local Menu options or on any menu, use HELP when\n\
the menu appears.\n"

BACK_SPACE1	"Erases the character to the left of the cursor."
BACK_SPACE2	"BACKSPACE erases the character to the left of the cursor and moves\n\
the cursor left one space.  It is usually used to correct a typing\n\
mistake.  When the editor is in INSERT-MODE, any text entered is\n\
inserted, it does not overwrite existing text.  In INSERT-MODE, BACKSPACE\n\
moves the text to the right of the cursor one place to the left."

BEGIN_LINE1	"Moves the cursor to the beginning of the line."
BEGIN_LINE2	"BEGIN-LINE moves the cursor to the first character that is not a space on\n\
the current line.  The window is shifted to the left if necessary.  If\n\
there are no characters on the line, the cursor is positioned in the first\n\
column.\n\
\n\
The sequence ENTER BEGIN-LINE starts a box region.  The original cursor\n\
position is the start of the region, and the cursor is moved to the\n\
beginning of the line. The ****BOX/LINE**** banner appears at the bottom of\n\
the screen.  Move the cursor so that the desired region is defined and use\n\
INSERT-LINE, PICK-UP, PICK-COPY or DELETE-LINE to insert, remove, copy, or\n\
delete the region."

BOX_MARK1	"Defines a box or line region."
BOX_MARK2	"BOX-MARK defines a box or line region.  Once the region is defined, use\n\
INSERT-LINE, PICK-UP, PICK-COPY or DELETE-LINE to insert, remove, copy, or\n\
delete the region.\n\
\n\
To define a box, move the cursor to one corner of the box and press\n\
BOX-MARK or ENTER.  Then move the cursor to the corner of the box\n\
diagonally opposite the first corner and use INSERT-LINE, PICK-UP,\n\
PICK-COPY, or DELETE-LINE. The ****BOX/LINE**** banner appears at the\n\
bottom of the screen when you are in box region or box line mode.\n\
\n\
To define a line region, move the cursor on the top or bottom line of the\n\
region and use BOX-MARK or ENTER.  Move the cursor to the end of the\n\
region, keeping the cursor in the same column and press INSERT-LINE,\n\
PICK-UP, PICK-COPY, or DELETE-LINE.\n\
\n\
When defining a region, use the arrow function keys, BEGIN-LINE, END-LINE,\n\
TAB, BACKTAB, PAGE-DOWN, PAGE-UP, LINES-DOWN, LINES-UP, RIGHT, LEFT, HOME,\n\
and RETURN to move the cursor and window."

BREAK1		"Stops the current search or filter."
BREAK2		"BREAK stops searches or filters.  A message appears, indicating that the\n\
operation has been interrupted.  If the operation has already completed,\n\
the BREAK will have no effect.\n\
\n\
To determine which key acts as BREAK, exit from INed, and run the following\n\
command:\n\
     stty -a\n\
The BREAK key is labeled with:\n\
    intr=\n\
For example, if the BREAK key is Ctrl-C, the following will be shown:\n\
    intr=^c"
$ Do not translate "stty -a", "intr=", or "intr=^c".

CANCEL1		"Removes menus, error messages, and popup boxes."
CANCEL2		"CANCEL removes popup boxes, menus, and error messages.  CANCEL also\n\
cancels regions after you use BOX-MARK or TEXT-MARK."

CENTER1		"Centers the line under the cursor between margins."
CENTER2		"CENTER centers the line that the cursor is on between the left and right\n\
margins. If no margins are in effect, the line is centered in the field.\n\
The sequence ENTER Number CENTER centers the specified number of lines,\n\
starting at the cursor line.\n\
\n\
CENTER can also be used in box or line region mode.  In this case, all\n\
lines in the region will be centered between the current margins."

DELETE_LINE1	"Deletes the line the cursor is on from the text."
DELETE_LINE2	"DELETE-LINE removes the specified region or the line that the cursor is on\n\
from the text.  Once text is removed, it can be restored using RESTORE.  To\n\
remove a specific number of lines, use the sequence ENTER Number\n\
DELETE-LINE, where Number is the number of lines to remove."

DELETE_CHAR1	"Deletes the character under the cursor."
DELETE_CHAR2	"DELETE-CHARACTER deletes the character at the cursor from the text.  If\n\
INed is in INSERT mode, the rest of the characters on the line are moved\n\
one position to the left.  If INed is in OVERWRITE mode, the rest of the\n\
characters on the line are left where they are, and the deleted character\n\
is replaced with a space.\n\
\n\
The sequence ENTER DELETE-CHARACTER deletes all characters from the cursor\n\
to the end of the line.  These characters are saved and can be restored\n\
with the RESTORE command."

DO1		"Runs text through a filter command."
DO2		"DO performs special processing on the current text.  The text up to the\n\
next blank line is sent to any shell command.  The shell command can\n\
include pipes and shell variables.  For example,\n\
\tENTER rpl pattern1 pattern2 | sort DO\n\
replaces pattern1 with pattern2 and sorts the result.\n\
RESTORE restores the text replaced by DO.\n\
The sequence ENTER Command DO runs the command on the rest of the current\n\
paragraph.  DO runs the last filter on the rest of the current paragraph.\n\
ENTER DO prints out the last DO command.\n\
\n\
The sequence ENTER Number Command DO sends the next Number paragraphs to\n\
the shell command.  Note that there is a space between the number and the\n\
command.  If the letter l is appended to the number, the specified number\n\
of lines, rather than paragraphs is used."
$ Do not translate rpl, sort, or the single character 'l' (in "If the
$ letter l is appended...")

DOWN_ARROW1	"Moves the cursor down one line."
DOWN_ARROW2	"DOWN-ARROW moves the cursor down 1 line in the current field.  If the\n\
cursor is on the last line of the field, the down arrow key moves the\n\
cursor off the field and down the screen.\n\
\n\
The sequence ENTER DOWN-ARROW creates box or line region mode.  The start\n\
of the region is the original cursor position and the cursor is moved down\n\
one line.  The ****BOX/LINE**** banner appears at the bottom of the screen.\n\
Move the cursor to diagonally the opposite corner of the desired box region\n\
or to the last line in the desired line region, then use INSERT-LINE,\n\
PICK-UP, PICK-COPY or DELETE-LINE to open, remove, copy, or delete the\n\
region."

END_LINE1	"Moves the cursor to the end of the current line."
END_LINE2	"END-LINE moves the cursor to the first space character after the end of the\n\
current line.  The window is shifted to the left if necessary.  If there\n\
are no characters on the line, the cursor is positioned in the first\n\
column.\n\
\n\
The sequence ENTER END-LINE creates box orline region mode.  The start of\n\
the region is the original cursor position and the cursor is moved to the\n\
end of the line. The ****BOX/LINE**** banner appears at the bottom of the\n\
screen.  Move the cursor to the opposite corner of the desired box or\n\
to the last line of the desired line region.  Press then use INSERT-LINE,\n\
PICK-UP, PICK-COPY, or DELETE-LINE to open, remove, copy, or delete\n\
the region."

ENTER1		"Enters a parameter for another editor command."
ENTER2		"ENTER displays a popup box into which you can type.  Use the popup box to\n\
provide parameters to other commands.  These parameters can be alphabetic\n\
strings or numeric values. the parameter can also be empty by pressing\n\
ENTER and then the command, without typing anything into the popup box.\n\
Any parameter can contain environment variables.  For example, the sequence\n\
ENTER $HOME/file USE incorporates the $HOME environment variable. If the\n\
parameter is too long to fit into the ENTER box, the text is scrolled to\n\
the left to make room for more characters.\n\
\n\
To remove the ENTER popup box without running a command, press ENTER again\n\
or press CANCEL.  To insert the previous parameter typed into the ENTER\n\
box, press LAST-ARG.  This is useful when you have typed in a long\n\
parameter.  You can press LAST-ARG to insert the same parameter again.  You\n\
can also edit text in the ENTER box using BACKSPACE, the arrow functions,\n\
BEGIN-LINE and END-LINE, and INSERT-MODE."

EXECUTE1	"Selects the current menu entry."
EXECUTE2	"EXECUTE selects menu items and responds to editor questions. You can\n\
select menu options by moving the cursor to the desired option by using\n\
UP-ARROW and DOWN-ARROW and then pressing EXECUTE.\n\
To remove the menu without selecting an option, press CANCEL."

EXIT1		"Exits the editor."
EXIT2		"EXIT leaves the editor, saving all files.  If you entered the editor as\n\
part of your login procedure, EXIT also logs you out.  Otherwise, EXIT\n\
returns you to the system prompt.\n\
\n\
ENTER q EXIT exits without saving any changes made to any text files\n\
modified during editing session.  If you used SAVE in a file during the\n\
editing session, any changes made up to that point are saved.  Any changes\n\
made after the SAVE are lost."
$ Do not translate "ENTER q EXIT"

FONT1		"Changes the type font for underlining or graphics."
FONT2		"FONT changes the font of text as it is entered.  The sequence ENTER\n\
Letter FONT changes to the indicated font, where letter can be:\n\
\n\
  r  for Roman (the normal font)\n\
  w  for word underlining\n\
  c  for continuous underlining\n\
  g  for graphics characters\n\
\n\
On using FONT, the current font becomes the alternate font.  To switch to\n\
the alternate font, press FONT again with no parameter.  When the editor\n\
starts, the alternate font is set to continuous underlining.  By pressing\n\
FONT with no parameter you can start and stop underlining.\n\
\n\
The sequence ENTER FONT is the same as ENTER r FONT, which changes to the\n\
to Roman font."
$ "Roman" is the word used to describe the normal font that is used on
$ almost all computers.
$ Do not translate the individual characters 'r', 'w', 'c', or 'g'.

FORMAT1		"Formats the paragraph below the cursor."
FORMAT2		"FORMAT formats lines of text between the right and left margins,\n\
starting at the cursor line and ending at the first blank line.  The\n\
indentation of the cursor line is not changed and the rest of the lines are\n\
filled out to occupy the space between the margins."

GO_TO1		"Moves to the beginning or end of a file."
GO_TO2		"GO-TO moves to the first line of text.  If the cursor is already\n\
on the first line, GO-TO moves to the last line of text.\n\
\n\
The sequence ENTER GO-TO positions the cursor on the last line of text.\n\
\n\
The sequence ENTER Number GO-TO moves the cursor to the specified line\n\
number.   For example, to move to the fifth line of a file,\n\
press ENTER 5 GO-TO."

HELP1		"Gives help information."
HELP2		"HELP displays information about the current editing situation.\n\
You can press HELP at any time.  If you are editing a text file, a\n\
menu of help information appears.  If you are being asked a question,\n\
HELP displays a box with more information about what is required.\n\
If you are looking at an error message, HELP displays a message that\n\
explains the error in more detail.  If you are looking at a menu,\n\
HELP explains the menu items in more detail."

HOME1		"Moves the cursor to the upper left corner of the text."
HOME2		"HOME moves the cursor to the upper left corner of the screen.\n\
\n\
The sequence ENTER HOME creates box or line region mode.  The start of\n\
the region is the original cursor position and the cursor is moved to the\n\
upper left corner of the field.  The ****BOX/LINE**** banner appears at\n\
the bottom of the screen.  Move the cursor to the diagonally opposite\n\
corner of the desired box region and press INSERT-LINE, PICK-UP,\n\
PICK-COPY, or DELETE-LINE to open, remove, copy, or delete the region."

INSERT_LINE1	"Inserts a new blank line before the current line."
INSERT_LINE2	"INSERT-LINE inserts a new blank line in the text above the line\n\
on which the cursor rests.\n\
\n\
The sequence ENTER Number INSERT-LINE inserts the specified number of\n\
blank lines.\n\
\n\
INSERT-LINE can also be used in box, line, or text region mode to insert\n\
an empty space the size of the region into the file."

INSERT_MODE1	"Switches between insert and overwrite modes."
INSERT_MODE2	"INSERT-MODE changes the typing mode between insert and overwrite.\n\
\n\
When insert mode is active, any entered text is inserted in the file.  In\n\
insert mode, BACK-SPACE moves text to the right of the cursor so that\n\
the text follows the backspacing cursor.\n\
\n\
When overwrite mode is active, any text entered into the file erases existing\n\
text, and BACKSPACE does not move text to the right of the cursor."

LAST_ARG1	"Restores the last parameter typed into the ENTER box."
LAST_ARG2	"LAST-ARG is used after ENTER, to display the last parameter that\n\
was typed into the ENTER box.  For example, if you want to search for\n\
a string but used the sequence ENTER String RETURN instead of\n\
ENTER String SEARCH-DOWN, you could use LAST-ARG to restore the string\n\
you typed in.  Press SEARCH-DOWN to start the search."

LEFT1		"Displays text that is left of the window."
LEFT2		"LEFT moves the window to the left.  If RIGHT has been used to\n\
move the window to the right, press LEFT to move the window to the\n\
left about a third of the width of the window.\n\
\n\
The sequence ENTER Number LEFT moves the window left by the specified\n\
number of columns."

LEFT_ARROW1	"Moves the cursor 1 column to the left."
LEFT_ARROW2	"LEFT-ARROW moves the cursor left 1 column in the current field.\n\
If the cursor is on the first column of the field, LEFT-ARROW moves it\n\
off the field. \n\
\n\
The sequence ENTER LEFT-ARROW creates box or line region mode.  The start\n\
of the region is the original cursor position and the cursor is moved to\n\
the left one space. The ****BOX/LINE**** banner appears at the bottom of\n\
the screen.  Move the cursor to the diagonally opposite corner of the\n\
desired box region or to the last line in the desired line region. Press\n\
INSERT-LINE, PICK-UP, PICK-COPY, or DELETE-LINE to open, remove, copy,\n\
or delete the region."

LINE_FEED1	"Moves the cursor to the next field."
LINE_FEED2	"LINE-FEED moves the cursor to the farthest left  field below the current\n\
field.  LINE-FEED is similar to RETURN, except that LINE-FEED always changes\n\
to another field, while RETURN stays in the same data field in which the\n\
cursor is positioned."

LINES_DOWN1	"Moves the window several lines toward the end of the file."
LINES_DOWN2	"LINES-DOWN moves the window toward the end of the file by a few lines.\n\
The number of lines is about a third of the height of the current field.\n\
\n\
The sequence ENTER Number LINES-DOWN moves the window the specified number\n\
of lines.\n\
\n\
The sequence ENTER LINES-DOWN positions the cursor on the first line of\n\
the window."

LINES_UP1	"Moves the window several lines toward the beginning of the file."
LINES_UP2	"LINES-UP moves the window toward the beginning of the file by a\n\
few lines. The number of lines is about a third of the height of the\n\
current field.\n\
\n\
The sequence ENTER Number LINES-UP moves the window by the specified\n\
number of lines.\n\
\n\
The sequence ENTER LINES-UP puts the cursor in the last line of the window."

LOCAL_MENU1	"Displays the menu of functions for the specific kind of displayed data."
LOCAL_MENU2	"LOCAL-MENU displays a menu of any special functions available for\n\
the current file.  You can then select the function you want to perform\n\
by moving the cursor with the arrow keys and pressing EXECUTE, or by\n\
invoking the function key (1) through (8) that corresponds to the option.\n\
LOCAL-MENU is available only in some editing situations.\n\
\n\
Each menu item has a number in parenthesis at the beginning of the\n\
line. This is the number of the function key associated with that\n\
special function. To perform a function, use the corresponding function\n\
key with or without displaying the menu.\n\
\n\
To receive more information on menu entries, press HELP while the menu\n\
is displayed."

MARGIN1		"Sets and clears margins."
MARGIN2		"MARGIN sets the left margin to the column below the cursor.  The\n\
initial margins are determined by the form being used to view data.\n\
On some forms, such as the standard text form, the tabs and margins\n\
are displayed on the first line of the screen.  If the margins are\n\
displayed, the left margin is indicated by the letter l and the right\n\
margin by the letter r.\n\
\n\
The sequence ENTER MARGIN sets the right margin to the column below\n\
the cursor.\n\
\n\
To remove the margins, use the sequence ENTER MARGIN with the cursor on\n\
the left margin."

MENU1		"Displays the New Task Menu."
MENU2		"MENU displays the New Task Menu, which lists a variety of tasks.\n\
These may involve editing a new file, running programs, or changing\n\
helpers.  The items in this menu are taken from your editor profile file.\n\
If you do not have an editor profile file, a system default version is\n\
used.  To learn how to create your own editor profile,press HELP and\n\
select the option How to Make an Editor Profile.  This help text describes\n\
in detail how to customize your editing environment with your editor\n\
profile file, and how to set up the menu items for your New Task Menu."

NEXT1		"Displays the next item in the current list."
NEXT2		"NEXT looks at the next item in the current list.  NEXT\n\
is available only in some editing situations.\n\
\n\
The sequence ENTER Number NEXT moves to the specified item number.\n\
The first item in the list is item number zero."

NEXT_WINDOW1	"Moves the cursor to the next field or window."
NEXT_WINDOW2	"NEXT-WINDOW moves the cursor to the next field or window in reading\n\
order. Reading order is left to right and top to bottom. Use this after\n\
you have pressed WINDOW to create multiple windows on the screen.\n\
\n\
The sequence ENTER NEXT-WINDOW moves the cursor to the previous\n\
field or window."

PAGE_DOWN1	"Displays the next screen of text."
PAGE_DOWN2	"PAGE-DOWN moves 1 screen at a time toward the end of the field.\n\
\n\
The sequence ENTER Number PAGE-DOWN moves the specified number of screens.\n\
\n\
The sequence ENTER PAGE-DOWN enters box or line region mode.  The start\n\
of the region is the original cursor position and the cursor is moved\n\
to the next screen. The ****BOX/LINE**** banner appears at the bottom of\n\
the screen.  Move the cursor to the diagonally opposite corner of the\n\
desired box region or to the last line in the desired line region and press\n\
INSERT-LINE, PICK-UP, PICK-COPY, or DELETE-LINE to open, remove, copy, or\n\
delete the region."

PAGE_UP1	"Displays the previous screen of text."
PAGE_UP2	"PAGE-UP moves 1 screen at a time toward the beginning of the field.\n\
\n\
The sequence ENTER Number PAGE-UP moves the specified number of screens.\n\
\n\
The sequence ENTER PAGE-UP enters box or line region mode.  The start of\n\
the region is the original cursor position and the cursor is moved to the\n\
previous screen. The ****BOX/LINE**** banner appears at the bottom of the\n\
screen.  Move the cursor to the diagonally opposite corner of the desired\n\
box region or to the last line in the desired line region and press\n\
INSERT-LINE, PICK-UP, PICK-COPY, or DELETE-LINE to open, remove, copy,\n\
or delete the region."

PICK_COPY1	"Picks up a copy of the line that the cursor is on."
PICK_COPY2	"PICK-COPY picks up a copy of the line that the cursor is on.  To move\n\
the copy to another position, move the cursor and press either PUT-COPY\n\
or PUT-DOWN. PUT-COPY puts a copy of the text only. You can use PUT-COPY\n\
repeatedly to put multiple copies of the text.  PUT-DOWN puts the text\n\
down. Once it is put with PUT-DOWN, it cannot be put again unless it is\n\
picked again. For example, to copy 2 lines from one place to another, put\n\
the cursor on the first line and press PICK-COPY twice.  Then move the\n\
cursor to the new location and press PUT-DOWN twice.\n\
\n\
PICK-COPY can also pick up a copy of the current region in box,line, or\n\
text region mode.  The region can then be copied to another location\n\
with the PUT-DOWN or PUT-COPY command.\n\
\n\
The sequence ENTER Number PICK-COPY picks the specified number of lines\n\
at once.  For example, to copy several lines, put the cursor on the first\n\
line, then press ENTER.  Enter the number of lines you want to copy, and\n\
press PICK-COPY.  You can then move the cursor to the new location and press\n\
PUT-DOWN or PUT-COPY.  Press PUT-COPY if you want to put copies of the text\n\
in several locations."

PICK_UP1	"Picks up the line the cursor is on."
PICK_UP2	"PICK-UP picks up the line the cursor is on and removes it from the file.\n\
To move the text to another position, move the cursor and press either\n\
PUT-COPY or PUT-DOWN.  PUT-COPY puts a copy of the text only. You can press\n\
PUT-COPY repeatedly to put multiple copies of the text.  PUT-DOWN puts\n\
the text down. Once the text is put with PUT-DOWN, it cannot be put again\n\
unless it is picked again.  For example, to move 2 lines from one place\n\
to another, put the cursor on the first line and press PICK-UP twice.\n\
Then move the cursor to the new location and press PUT-DOWN twice.\n\
\n\
PICK-UP can also pick up the current region in box,line, or text region\n\
mode.  The region can then be copied to another location with the \n\
PUT-DOWN or PUT-COPY command.\n\
\n\
The sequence ENTER Number PICK-UP picks the specified number of lines at\n\
once.  For example, to copy several lines, put the cursor on the first\n\
line, then press ENTER.  Enter the number of lines you want to pick up, and\n\
press PICK-UP.  You can then move the cursor to the new location and press\n\
PUT-DOWN or PUT-COPY.  Press PUT-COPY if you want to put copies of the text\n\
in several locations."

PREVIOUS1	"Displays the previous item in the current list."
PREVIOUS2	"PREVIOUS looks at the previous item in the current list.\n\
PREVIOUS can be used only in some editing situations.\n\
\n\
The sequence ENTER Number PREVIOUS moves to the specified item number.\n\
The first item in the list is item number zero."

PRINT1		"Displays a menu of print options."
PRINT2		"PRINT displays a menu of the print options that are used to print the\n\
file you are viewing.  You can customize your Print Menu by editing your\n\
print profile file, which $HOME/profiles/printprf."
$ Do not translate $HOME/profiles/printprf

PUT_COPY1	"Inserts a copy of the text most recently picked."
PUT_COPY2	"PUT-COPY inserts at the cursor position a copy of the text most\n\
recently picked with PICK-COPY or PICK-UP.  Pressing PUT-COPY repeatedly\n\
puts multiple copies of the same text.  For example, to copy a line to\n\
three new locations, press PICK-COPY to copy the line, move the cursor\n\
to the first location and press PUT-COPY. Move the cursor to the second\n\
location and press PUT-COPY.  Then move the cursor to the third location\n\
and press PUT-DOWN.\n\
\n\
To move 2 lines press PICK-UP twice.  You can then move the cursor to the\n\
new location and press PUT-DOWN twice."

PUT_DOWN1	"Inserts the text most recently picked."
PUT_DOWN2	"PUT-DOWN inserts at the cursor position the text most recently\n\
picked with PICK-COPY or PICK-UP.  Pressing PUT-DOWN repeatedly inserts\n\
text into the file in the reverse order that it was picked.  For\n\
example, if you picked up a line containing 1, then another line\n\
containing 2, then another line containing 3, using PUT-DOWN\n\
repeatedly would insert line 3, then line 2, then line 1. The\n\
resulting text would be in the same order it was in when picked."

QUIT1		"Terminate the editing session if no other function works."
QUIT2		"QUIT ends an editing session if the normal INed functions are\n\
not working properly.  Use QUIT only if no other function works.  When\n\
you use QUIT, the editor tries to save files, then exits.\n\
\n\
To find out which key or sequence of keys performs the QUIT, exit from\n\
INed, and run the command\n\
    stty -a\n\
The QUIT key is labeled with\n\
    quit=\n\
For example, if the BREAK key is ctrl-^, the following will be shown:\n\
    quit=^"
$ Do not translate stty -a, quit, ctrl-^, or quit=^

QUOTE1		"Enters a graphics character into the text."
QUOTE2		"QUOTE enters a single graphics character into the text.\n\
To enter a single graphics character, press QUOTE and type the\n\
character.  The graphics character corresponding to the text\n\
character is inserted into the text.  This is the same as using\n\
the FONT command to change to the graphics font, typing the text\n\
character, and using the FONT command to return to the original font."

REFRESH1	"Clears and redraws the screen."
REFRESH2	"REFRESH clears and redraws the screen.  This is useful if line\n\
noise or a system message has disturbed your screen."

REPLACE1	"Replaces a phrase found by SEARCH with another phrase."
REPLACE2	"The sequence ENTER String REPLACE replaces the current search string\n\
with the specified replacement string. After defining the string, you can\n\
press REPLACE without specifying a string.  If the cursor is not on the\n\
first character of the search string, REPLACE does nothing.\n\
\n\
REPLACE can repeatedly change one phrase in the text to another.\n\
Press SEARCH-DOWN or SEARCH-UP to locate the phrase to be replaced,\n\
then press REPLACE to substitute the new phrase.  Both SEARCH and\n\
REPLACE remember their parameter strings, so you can then use the\n\
sequence SEARCH-DOWN REPLACE repeatedly to find new occurrences of\n\
an old phrase and to substitute it with a new phrase."

RESTORE1	"Restores the last item deleted."
RESTORE2	"RESTORE restores the last text that was deleted.  The text may\n\
have been removed with DELETE-LINE, or with other functions that \n\
delete text such as FORMAT, DO, and ENTER DELETE-CHARACTER.\n\
\n\
To restore text that was deleted earlier in the session, edit the\n\
file named # by using the sequence ENTER # USE.  This displays all\n\
text that you have deleted since you started your editing session.\n\
You can then move the cursor to the text you want to restore and press\n\
PICK-COPY to copy it, USE to return to your original file, then\n\
PUT-DOWN or PUT-COPY to restore the text."

RETURN1		"Moves the cursor to the next line."
RETURN2		"RETURN moves the cursor to the beginning of the next line of the\n\
current field.  If the next line is empty, the cursor automatically\n\
indents to the same column as the first character on the line above\n\
it.  If the line is not empty, the cursor is positioned on the first\n\
character that is not blank.\n\
\n\
The sequence ENTER RETURN enters box or line region mode.  The start of\n\
the region is the original cursor position and the cursor is moved to the\n\
beginning of the next line.  The ****BOX/LINE**** banner appears at the\n\
bottom of the screen.  Move the cursor to the diagonally opposite corner\n\
of the desired box region or to the last line in the desired line region\n\
and press INSERT-LINE, PICK-UP, PICK-COPY, or DELETE-LINE to open, remove,\n\
copy, or delete the region."

RIGHT1		"Displays text that is to the right of the window."
RIGHT2		"RIGHT moves the window to the right about a third of the width of\n\
the window.  The window can be moved back with the LEFT command.  RIGHT\n\
creates or reads material that is wider than the window.\n\
\n\
The sequence ENTER Number RIGHT moves the window left by the specified\n\
number of columns."

RIGHT_ARROW1	"Moves the cursor one column to the right."
RIGHT_ARROW2	"RIGHT-ARROW moves the cursor right 1 column in the current\n\
field. If the cursor is on the last column  of the field, RIGHT-ARROW\n\
moves it off the field. \n\
The sequence ENTER RIGHT-ARROW enters box or line region mode.  The\n\
start of the region is the original cursor position and the cursor is\n\
moved to the right one space. The ****BOX/LINE**** banner appears at the\n\
bottom of the screen.  Move the cursor to the diagonally opposite corner\n\
of the desired box region or to the last line in the desired line region\n\
and press INSERT-LINE, PICK-UP, PICK-COPY, or DELETE-LINE to open, remove,\n\
copy, or delete the region."

SAVE1		"Saves the current state of the file being edited."
SAVE2		"SAVE saves the current file in its current state.  Press SAVE\n\
periodically when editing text files.  If you are working on a text\n\
file, SAVE saves the old copy of the file with the  extension .bak\n\
appended to the old file name.\n\
\n\
The sequence ENTER Filename SAVE saves the current file and then copies\n\
the file to the specified file name.  If a file of that name already exists,\n\
the existing file is backed up so that it is not lost.\n\
\n\
The sequence ENTER SAVE is the same as ENTER File SAVE except that the\n\
file name is taken from the text at the current cursor position.  The\n\
name is the string of characters starting at the cursor position and\n\
continuing until the first space character."
$ Do not translate .bak

SEARCH_DOWN1	"Searches forwards in the text for a specified phrase."
SEARCH_DOWN2	"The sequence ENTER String SEARCH-DOWN searches forward in the text\n\
for the specified string.  The cursor moves to the first occurrence of the\n\
string.  To continue searching forward in the file for that string, press\n\
SEARCH-DOWN again, without specifying a string.\n\
\n\
The sequence ENTER SEARCH-DOWN gets the search string from the text at the\n\
current cursor position.  The search string is the string of characters\n\
starting at the cursor position and continuing until the first space character."

SEARCH_UP1	"Searches backward in the text for a specified phrase."
SEARCH_UP2	"The sequence ENTER String SEARCH-UP searches backward in the text for\n\
the specified string.  The cursor moves to the first occurrence of the\n\
string.  To continue searching backward in the file for the string, press\n\
SEARCH-UP again, without specifying a string.\n\
\n\
The sequence ENTER SEARCH-UP gets the search string from the text at the\n\
current position.  The search string is the string of characters starting\n\
at the cursor position and continuing until the first space character."

SET_TAB1	"Sets and clears tab stops."
SET_TAB2	"SET-TAB sets a tab stop in the column at the current cursor position.\n\
\n\
The sequence ENTER SET-TAB clears the tab stop in the cursor column.\n\
\n\
Tabs can be set only if tab stops are allowed in the text field in\n\
which the cursor is positioned.  Some displays contain a tab rack, in\n\
which the tabs and margins are displayed on the first line of the\n\
screen.  If so, the tabs stops are indicated by the letter t."

TAB1		"Moves the cursor right to the next tab stop or next field."
TAB2		"TAB moves the cursor to the right to the next tab stop.  If there is no\n\
tab stop to the right, TAB moves to the next data field to the right.\n\
\n\
The sequence ENTER TAB enters box or line region mode.  The start of the\n\
region is the original cursor position and the cursor is moved to the next\n\
tab stop.  The ****BOX/LINE**** banner appears at the bottom of the\n\
screen.  Move the cursor to the diagonally opposite corner of the desired\n\
box region or to the last line in the desired line region and press\n\
INSERT-LINE, PICK-UP, PICK-COPY, or DELETE-LINE to open, remove, copy,\n\
or delete the region."

BACKTAB1	"Moves the cursor left to the next tab stop or next field."
BACKTAB2	"BACKTAB moves the cursor left to the previous tab stop.  If there is\n\
no tab stop to the left, BACKTAB moves to the previous field.\n\
\n\
The sequence ENTER BACKTAB enters box or line region mode.  The start of\n\
the region is the original cursor position and the cursor is moved to the\n\
previous tab stop.  The ****BOX/LINE**** banner appears at the bottom of\n\
the screen.  Move the cursor to the diagonally opposite corner of the\n\
desired box region or to the last line in the desired line region and press\n\
INSERT-LINE, PICK-UP, PICK-COPY, or DELETE-LINE to open, remove, copy,\n\
or delete the region."

TEXT_MARK1	"Defines a text region."
TEXT_MARK2	"TEXT-MARK defines a text region.  This type of region is used to\n\
move or copy sentences.  INSERT-LINE, PICK-UP, PICK-COPY, or DELETE-LINE\n\
commands can then insert, remove, copy, or delete the region.\n\
\n\
To define a text region, move the cursor to top of the region and\n\
press TEXT-MARK.  The ******TEXT****** banner appears at the bottom of\n\
the screen.  Move the cursor 1 space to the right of the end of the\n\
desired region and press INSERT-LINE, PICK-UP, PICK-COPY, or\n\
DELETE-LINE.  The cursor position is not part of the region.\n\
\n\
While in region mode, you can use the arrow functions, BEGIN-LINE,\n\
END-LINE, TAB, BACKTAB, PAGE-DOWN, PAGE-UP, LINES-DOWN, LINES-UP,\n\
RIGHT, LEFT, HOME, and RETURN to move the cursor and the window."

UP_ARROW1	"Moves the cursor up 1 line."
UP_ARROW2	"UP-ARROW moves the cursor up 1 line in the current field.\n\
\n\
The sequence ENTER UP-ARROW enters box or line region mode.  The start\n\
of the region is the original cursor position and the cursor is moved up\n\
1 line.  The ****BOX/LINE**** banner appears at the bottom of the screen.\n\
Move the cursor to the diagonally opposite corner of the box or to the\n\
last line in the region and press INSERT-LINE, PICK-UP, PICK-COPY, or\n\
DELETE-LINE to open, remove, copy, or delete the region."

USE1		"Displays a new file."
USE2		"The sequence ENTER File USE edits a new file.  If the file does not\n\
exist, the editor displays the file creation menu so that you can create\n\
the file.  Once you have switched to a new file, you can press USE\n\
repeatedly to switch between the 2 files."

WINDOW1		"Makes a new window."
WINDOW2		"WINDOW makes a new window on the screen.  It can only be\n\
used when you are editing a text file, and the new window must also\n\
contain a text file.  WINDOW creates a new window and puts the current\n\
file in it.  The sequence ENTER File WINDOW creates a new window that\n\
contains the specified file.\n\
\n\
The position of the new window is determined by the cursor position at\n\
the time you press WINDOW.  If the cursor is on the top line of the\n\
current window, a vertical window is created. The current window is\n\
split by a vertical line going through the current cursor position,\n\
and the new window is the area to the right of this line.  If the\n\
cursor is not on the top line of the window, a horizontal window is\n\
created. The current window is split by a horizontal line going through\n\
the cursor, and the new window is the area below this line."

ZOOM_IN1	"Zooms in to show more detail about the item the cursor is on."
ZOOM_IN2	"ZOOM-IN shows more detail about the item the cursor is on.  ZOOM-IN\n\
can be used in some editing situations only.  It does not take any\n\
parameters.  Use the arrow functions to move the cursor to an item,\n\
then press ZOOM-IN.  To return to the original location, press ZOOM-OUT."

ZOOM_OUT1	"Zooms out to show less detail about the current item."
ZOOM_OUT2	"ZOOM-OUT shows less detail about the current item.  It does\n\
not take any parameters.  Use ZOOM-OUT to display less detail about\n\
the current information on the screen."
