$ @(#)48	1.1  src/tenplus/helpers/editorprf/INeditorprf.msg, tenplus, tenplus411, GOLD410 3/17/93 14:02:58
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
$set MS_INEDITORPRF 
$
5000 "Dummy message to add Helper message offset."
$ Translator: do not translate the above message at all
$
$ TRANSLATOR:  In this file, many command (function) names are used.
$ They are all in UPPER CASE.  They should not be translated.
$ Examples:  BOX-MARK, ENTER, TAB, RIGHT-ARROW, and many more.
$
$ Translator:  do not translate anthing that looks like %s or %d or
$ %1$s or %2$d etc.
$
$ Translator: do not translate "#<number>" at the end of messages
$
N_ARGNUMBER "A numeric parameter is out of range.#1"

H_N_ARGNUMBER "0611-351 You tried to move to another of the editor profile options, but\n\
you specified a number larger than the number of options.  To move\n\
to another option:\n\
 o use NEXT or PREVIOUS several times, as necessary;\n\
 o specify a numeric parameter that is not too large; or\n\
 o use ZOOM-OUT, move the cursor to the desired option, and then use\n\
   ZOOM-IN.\n\
\n\
Press CANCEL to continue."
$
N_ARGBAD "The parameter must be a number.#1"

H_N_ARGBAD "0611-352 You used the sequence ENTER Parameter NEXT or PREVIOUS to move to\n\
another editor profile option, but specified a parameter that was not a\n\
number.  Specify a number with the sequences ENTER Parameter NEXT or ENTER\n\
Parameter PREVIOUS only.  To move to another option:\n\
 o use NEXT or PREVIOUS as many times as necessary;\n\
 o specify a numeric parameter that is not too large; or\n\
 o use ZOOM-OUT, move the cursor to the desired option and then use\n\
   ZOOM-IN.\n\
\n\
Press CANCEL to continue."
$
N_BADPROF "Cannot open profile file %s.#1"

H_N_BADPROF "0611-353 You tried to edit a profile file but the system cannot open\n\
the file.  Use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$
N_DEBUG "Cannot open Debugging Output File."
$
N_FILECREATE "Cannot create file %s.#1"

H_N_FILECREATE "0611-354 The system cannot copy the system version of the profile file into\n\
the current directory.  Check directory permissions or use local problem\n\
reporting procedures.\n\
\n\
Press CANCEL to continue."
$
N_FILEOPEN "Cannot open file %s for reading.#1"

H_N_FILEOPEN "The system cannot open the system version of the profile file.\n\
Use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$
N_FORM "Cannot find the %s form.#1"

H_N_FORM "The system cannot find a form it needs to display profile\n\
information.  Use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$
N_NEXT "There are no more profile items.#1"

H_N_NEXT "You used NEXT but you are viewing the last item in the editor profile.\n\
There is no next item.  To move to the preceding item, use PREVIOUS.\n\
\n\
Press CANCEL to continue."
$
N_LIPROFILE "Cannot open the system profile file %1$s\n\
that is specific to language.\n\
Using system profile file %2$s\n\
that is independent of language.\n\
\n\
Press Press CANCEL to continue."
$
N_UINIT "Copying the standard profile %s."
$
N_ZOOMIN "Do not use ZOOM-IN on a blank line.#1"

H_N_ZOOMIN "You used ZOOM-IN when the cursor was on a blank line in the editor profile\n\
display of options.  This is not allowed, because there is no profile\n\
information associated with this line.  Move the cursor to a line that is\n\
not blank, then use ZOOM-IN.\n\
\n\
Press CANCEL to continue."
$
N_BADWRITE "Cannot copy the system editor profile.#1"

H_N_BADWRITE "The system cannot copy the system version of the editor profile.\n\
Check that there is enough disk space or use local problem reporting\n\
procedures.\n\
\n\
Press CANCEL to continue."
$
COLORLIST "black\nblue\ngreen\ncyan\nred\nmagenta\nbrown\nwhite#0"
$
$ Do not translate the '\n', or #0
$ cyan is a greenish-blue color
$ magenta is a purple-red color
$
LINE1 "Modify only the first line of these fields.#1"

H_LINE1 "You tried to modify some line other than the first line.  This is not\n\
allowed, as only the first lines of these fields will be examined to\n\
determine color selections.  Modify only the first line.\n\
\n\
Press CANCEL to continue."
$
COLORFRM "The color form is missing field %s.#1"

H_COLORFRM "The pcolor form has been damaged.\n\
Use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$ Do not translate "pcolor".
$
FIELDNAME "Field name %s is not valid.#1"

H_FIELDNAME "The pcolor form has been damaged.\n\
Use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$ Do not translate "pcolor".
$
BADCOLORLIST "The color list in the editor profile help message file is not correct.#1"

H_BADCOLORLIST "The editor profile help message file has been damageed.\n\
Use local problem reporting procedures.\n\
\n\
Press CANCEL to continue."
$
UNK_COLOR "The selected color is not known.#1"

H_UNK_COLOR "You specified a color that is not a valid choice.\n\
Select a color from the list of colors.\n\
\n\
Press CANCEL to continue."
$
NOINSERT "These fields are limited to 1 line each.#1"

H_NOINSERT "You tried to insert multiple lines into this field.  This is not\n\
allowed, because only the first line of the field is examined for\n\
information about color.\n\
\n\
Press CANCEL to continue."
$
BADCOLORS "Warning: the selected color scheme will not distinguish between foreground,\n\
background, and underlined text.\n\
Press EXECUTE to accept the color scheme.\n\
Press CANCEL to restore the default color scheme.#1"

H_BADCOLORS "You selected a color scheme in which 1 of the following is true:\n\
\n\
    o the background and foreground colors are identical;\n\
    o the underline and underline background colors are identical or;\n\
    o the underline and foreground colors are identical.\n\
\n\
In the first 2 cases, the displayed text will not be visible.  In the third\n\
case, normal and underlined cannot not be distinguished.  To correct this,\n\
select colors so that the text is visible and so that underlined text can\n\
be distinguished from normal text.\n\
\n\
Press CANCEL to continue."
