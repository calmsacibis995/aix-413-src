$ ENGLISH_SOURCE_VERSION= 1.1
$ $*@(#)57      1.1  src/msg/bos/base/He.howtos.msg, bosmsg, msg411, GOLD410 2/9/94 08:45:11

$ $*COMPONENT_NAME: (INED) INed Editor

$ $*ORIGINS:  27

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

$quote "
$len
$set MS_HEHOWTOS

DEL_LINES1 "Remove 1 or more lines of text."
DEL_LINES2 "Approach 1\n\
\n\
Move the cursor to a line you want to remove, and press DELETE-LINE.\n\
Continue pressing the DELETE-LINE key until you have removed all lines you\n\
want to remove.\n\
\n\
Approach 2\n\
\n\
To remove a group of consecutive lines, use this sequence:\n\
  o Move the cursor to the first line you want to remove, and press the\n\
    ENTER key or the BOX-MARK key.  The ****BOX/LINE**** banner appears at\n\
    the bottom of the screen.\n\
  o Use the DOWN-ARROW key, the LINES-DOWN key, or the PAGE-DOWN key, or\n\
    any combination of these keys, to move the cursor to the last line you\n\
    want to remove.  Keep the cursor in the same column.\n\
  o Press the DELETE-LINE key."

COPY_LINES1 "Copy 1 or more lines of text from one place to another."
COPY_LINES2 "Approach 1\n\
\n\
Move the cursor to the first line you want to copy and press the PICK-COPY\n\
key.  Press the PICK-COPY key repeatedly until you have copied all of the\n\
lines you want to copy.  These lines can be located anywhere in the file,\n\
and do not have to be in consecutive order.  Move the cursor to the place\n\
in the text where you want to insert the copy of the lines and press the\n\
PUT-DOWN key once for each line you copied.\n\
\n\
Approach 2\n\
\n\
If you want to copy a group of consecutive lines, use this sequence:\n\
  o Move the cursor to the first line you want to copy and press the ENTER\n\
    key or the BOX-MARK key.  The ****BOX/LINE**** banner appears at the\n\
    bottom of the screen.\n\
  o Use the DOWN-ARROW key, the LINES-DOWN key, the PAGE-DOWN key, or any\n\
    combination of these keys, to move the cursor to the last line you want\n\
    to copy.  Keep the cursor in the same column.\n\
  o Press PICK-COPY.\n\
  o Move the cursor to the place in the text where the copy is to be\n\
    inserted and use the PUT-DOWN key or the PUT-COPY key.  (If you use the\n\
    PUT-COPY key, you can insert multiple copies by using the PUT-COPY key\n\
    repeatedly.)"

MOVE_LINES1 "Move 1 or more lines of text from one place to another."
MOVE_LINES2 "Approach 1\n\
\n\
Move the cursor to the first line you want to move, and press the PICK-UP\n\
key.  Press the PICK-UP key repeatedly until you have removed all of the\n\
lines you want to move.  These lines can be located anywhere in the file,\n\
and do not have to be in consecutive order.  Move the cursor to the place\n\
in the text where you want the copy of these lines to appear and press the\n\
PUT-DOWN key once for each line you copied.\n\
\n\
Approach 2\n\
\n\
If you want to move a group of consecutive lines, you may want to use this\n\
sequence:\n\
  o Move the cursor to the first line you want to move and press the\n\
    ENTER key or the BOX-MARK key.  The ****BOX/LINE**** banner appears at\n\
    the bottom of the screen.\n\
  o Use the DOWN-ARROW key, the LINES-DOWN key, the PAGE-DOWN key, or any\n\
    combination of these keys, to move the cursor to the last line you want\n\
    to move.  Keep the cursor in the same column.\n\
  o Press the PICK-UP key.\n\
  o Move the cursor to the place in the text where the text is to be\n\
    inserted and use the PUT-DOWN key or the PUT-COPY key.  If you use the\n\
    PUT-COPY key, you can insert multiple copies of the text by using the\n\
    PUT-COPY key repeatedly."

DEL_SENT1 "Remove a sentence."
DEL_SENT2 "You can use some combination of the DELETE-LINE key and the\n\
DELETE-CHARACTER key to remove every character in a sentence.  You can also\n\
remove the entire sentence at once by using this sequence:\n\
  o Position the cursor on the first character of the sentence, then press\n\
    the TEXT-MARK key.  The ******TEXT****** banner appears at the bottom\n\
    of the screen.\n\
  o Move the cursor to 1 character beyond the last character in the\n\
    sentence and press the DELETE-LINE key."

COPY_SENT1 "Copy a sentence from one place to another."
COPY_SENT2 "Make a copy of the sentence using this sequence:\n\
  o Position the cursor on the first character of the sentence, and press\n\
    the TEXT-MARK key.  The ******TEXT****** banner appears at th bottom of\n\
    the screen.\n\
  o Move the cursor to 1 character beyond the last character in the\n\
    sentence and press the PICK-COPY key.\n\
  o Move the cursor to the position in the text where you want to\n\
    insert a copy of the sentence and press the PUT-DOWN key or the\n\
    PUT-COPY key.  If you use the PUT-COPY key, you can insert multiple\n\
    copies of the sentence by pressing the PUT-COPY key repeatedly."

MOVE_SENT1 "Move a sentence from one place to another."
MOVE_SENT2 "Move a sentence using this sequence:\n\
  o Position the cursor on the first character of the sentence and press\n\
    the TEXT-MARK key.  The ******TEXT****** banner appears at the bottom\n\
    of the screen.\n\
  o Move the cursor to one character after the last character in the\n\
    sentence and press the PICK-UP key.\n\
  o Move the cursor to the position in the text where you want to insert a\n\
    copy of the sentence and press the PUT-DOWN key or the PUT-COPY key.\n\
    If you use the PUT-COPY, you can insert multiple copies of the sentence\n\
    by using the PUT-COPY key repeatedly."

MOVE_RIGHT1 "Move a column of text to the right."
MOVE_RIGHT2 "Move a column of text to the right using this sequence:\n\
  o Move the cursor to the first character you want to move and press\n\
    the BOX-MARK key.  The ****BOX/LINE**** banner appears at the bottom\n\
    of the screen.\n\
  o Use the DOWN-ARROW key, the LINES-DOWN key, the PAGE-DOWN key, or any\n\
    combination of these keys, to position the cursor on the last line you\n\
    want to move.\n\
  o Move the cursor right to the new position for the column.\n\
  o Press the INSERT-LINE key."

MOVE_LEFT1 "Move a column of text to the left."
MOVE_LEFT2 "Move a column of text to the left using this sequence:\n\
  o Move the cursor to the first character you want to move and press the\n\
    BOX-MARK key.  The ****BOX/LINE**** banner appears at the bottom of the\n\
    screen.\n\
  o Press the DOWN-ARROW key, the LINES-DOWN key, the PAGE-DOWN key, or any\n\
    combination of these keys, to move the cursor to the last line you want\n\
    to move.\n\
  o Move the cursor left to the new position for the column.\n\
  o Press the DELETE-LINE key.  This removes any characters that were in\n\
    the region, and moves the column to the left.  If you want to position\n\
    the column somewhere else, press the PICK-UP key instead of the\n\
    DELETE-LINE key."
