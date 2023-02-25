$ ENGLISH_SOURCE_VERSION= 1.4.2.5
$ @(#)12	1.1  src/msg/bos/base/dsga.msg, bosmsg, msg411, GOLD410 4/14/94 15:11:17
$
$   COMPONENT_NAME: DASGASL
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1991,1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$ 
$
$ GENERAL NOTES
$
$ Do not translate %d, %s, %x, %07X, or \t in any messages.  They
$   are used for word or number substitution and are noted in the
$   comments for the individual messages.  The 1$, 2$, 3$, etc,
$   within the substitutions are used to denote the order of the
$   substitutions.
$
$ These comments concern the TITLE LINES at the top the diagnostic screen.
$   The title must be in all capital letters. The first line
$   of the title cannot be longer than 70 characters starting from 
$   column 1.  If the line is greater than 70, it may be continued on
$   the next line.  Leave line spacing as shown:  one blank line after
$   the last title line.  For example:
$
$ *****
$ TESTING PORT 12 OF THE 16-PORT ASYNCHRONOUS ADAPTER IN PLANAR SLOT 2
$ IN ADVANCED MODE
$
$ Please stand by.
$ *****
$
$ These comments concern the user ACTIONS in all caps.
$   If translations require the creation of new lines, begin the
$   new lines in the column immediately following the row of periods. 
$   For example:
$
$ *****
$ ACTION.........one line of English might require several when translated, so
$                begin the next line at the same point of the previous line.
$ ACTION.........the next action follows with no blank line preceding it.
$ *****
$
$ END OF GENERAL NOTES


$quote "
$set LEGA_GENERIC

LEGA_TITLE "TESTING POWER Gt3 PROCESSOR %1$s IN LOCATION %2$s"
$
$ Diagnostics menu title.
$ See general notes on length of title.
$ %1$s is the name of the resource being tested.
$ %2$s is the location of the resource.
$ Leave line spacing as shown.  See general notes on length of title line.

LEGA_ADV "TESTING POWER Gt3 PROCESSOR %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE"
$
$ Diagnostics menu title in advanced mode.
$ See general notes on length of title.
$ %1$s is the name of the resource being tested.
$ %2$s is the location of the resource.
$ Leave line spacing as shown.  See general notes on length of title line.

LEGAZ_TITLE "TESTING POWER Gt3 PROCESSOR WITH Z-BUFFER %1$s IN LOCATION %2$s"
$
$ Diagnostics menu title.
$ See general notes on length of title.
$ %1$s is the name of the resource being tested.
$ %2$s is the location of the resource.
$ Leave line spacing as shown.  See general notes on length of title line.

LEGAZ_ADV "TESTING POWER Gt3 PROCESSOR WITH Z-BUFFER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE"
$
$ Diagnostics menu title in advanced mode.
$ See general notes on length of title.
$ %1$s is the name of the resource being tested.
$ %2$s is the location of the resource.
$ Leave line spacing as shown.  See general notes on length of title line.

LEGA_YES "YES"
$ This option is shown when a YES answer is possible.

LEGA_NO  "NO"
$ This option is shown when a NO answer is possible.

LEGA_ACTION "Move cursor to selection, then press Enter"
$ Users instruction on how to make a selection.

LEGA_STND_BY "Please stand by."

LEGA_L_STND_BY "Please stand by.\n\
This test may take up to 3 minutes."

LEGA_BLANKSC    "\nThis test may cause the screen to go blank for a while."

LEGA_NOTOUCH  "\nDo not press any key during this test."

LEGA_HAVEDISP	"\
Is there a monitor attached to the display adapter currently being tested?\n\n\
Move cursor to selection, then press Enter.\n\n\
NOTE:  If you choose YES, you will be asked to identify the COLORS of\n\
several screens.  If you do not wish to run any of the color display tests,\n\
choose NO."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

LEGA_LOOP_TITLE "TESTING POWER Gt3 PROCESSOR %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"

$
$ This panel is displayed when in the loop test mode.
$ %1$s is the name of the resource being tested.
$ %2$s is the location of the resource.
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

LEGA_PDPQ  "This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
WITHOUT exiting diagnostics or powering off the system unit:\n\n\
EXECUTE........the problem determination procedures that\n\
came with the display.\n\
ANSWER.........the question below.\n\n\
Did the problem determination procedures reveal a problem?\n\n\
Move cursor to selection, then press Enter."
$
$ See general notes on expanding ACTION lines if necessary.
$ Leave line spacing as shown.

LEGA_RP "FOLLOW.........the procedures for reporting problems with the display.\n\n\
Press Enter to continue."
$
$ See general notes on expanding ACTION lines if necessary.
$ %1$x is a six digit menu number that is displayed on the screen.
$ %2$s is a title message.  Be sure and leave these two substitutions
$ exactly as they are shown.

SGA_SIMMS "The system's configuration indicates that POWER Gt1\n\
graphics adapter video memory upgrade previously installed\n\
in the system is found to be missing.\n\
\n\
Has the video memory upgrade been removed from the system?"

LEGA_CBZ "The diagnostic tests have been executed and no trouble was found.\n\
For further problem determination, please refer to the 7008-M20 Service Guide\n\
for information on running the built in test of the display.\n\n\
Press Enter to continue."
$
$ See general notes on expanding ACTION lines if necessary.
$ %1$x is a six digit menu number that is displayed on the screen.
$ %2$s is a title message.  Be sure and leave these two substitutions
$ exactly as they are shown.

F3_EXIT  "\
This is a test of the display attached to the\n\
graphics adapter currently being tested.\n\n\
A %s color pattern will be displayed on the next screen.\n\
for 7 seconds.\n\n\
To display the pattern, press Enter."
$
$ %s is a color.  Be sure and leave this substitution
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

RED "red"
$
$ Red is the color red

GREEN "green"
$
$ Green is the color green

BLUE "blue"
$
$ Blue is the color blue

COLOR_SCREEN "\
This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
Was the color of the screen %s?\n\n\
Move cursor to selection, then press Enter."
$
$ %s is a color.  Be sure and leave this substitution
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
