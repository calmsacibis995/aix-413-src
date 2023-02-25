$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)65	1.1  src/msg/bos/base/INprint.msg, bosmsg, msg411, GOLD410 2/9/94 08:45:37
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
$set MS_INPRINT 
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
P_CURDIR "Cannot set the current directory to %s.#1"

H_P_CURDIR "0611-376 The Print Helper program cannot change its working directory.\n\
\n\
Press CANCEL to continue."
$
P_NOPROFILE "Cannot find the profile for the Print Helper.#1"

H_P_NOPROFILE "0611-377 The Print Helper program cannot locate the standard system\n\
profile of print options.\n\
\n\
Press CANCEL to continue."
$
P_STATEFILE "Cannot create the print statefile %s.#1"

H_P_STATEFILE "0611-378 The Print Helper program cannot create the /tmp/$LOGNAME file.\n\
The /tmp directory may not exist or it may have permissions that are\n\
not correct.\n\
\n\
Press CANCEL to continue."
$ Do not translate /tmp/$LOGNAME  or /tmp
$
P_NOMENU "There is no print options menu.#1"

H_P_NOMENU "0611-379 The Print Helper program cannot read the menu options from\n\
the print profile.  If there is a version of the print profile in\n\
$HOME/profiles/printprf, confirm that it contains data.  If it contains\n\
data, or if there is no such profile, contact your system administrator.\n\
\n\
Press CANCEL to continue."
$ Do not translate $HOME/profiles/printprf
$
MN_P_MENU "      Print Helper\n\
Move the cursor and press:\n\
EXECUTE to select,\n\
CANCEL to leave the menu, or\n\
HELP for more information.\n#1"
$
$ Do not translate the '\n', or #1
$ Center the first line above the remaining lines. Do not let
$ any of these lines go over 50 characters.

H_P_MENU "You used PRINT to display a menu of print options.  To customize the\n\
list of options, create and edit a structured file named\n\
$HOME/profiles/printprf.\n\
\n\
To select a menu option, move the cursor to one of the options and press\n\
EXECUTE.  Press CANCEL to remove the menu without selecting an option.\n\
\n\
Press CANCEL to continue."
$
P_CMD "Type the command to run (the default is %s):                      #1"
$ Please include the trailing spaces in this message.  The exact number of
$ spaces is not critical.

H_P_CMD "You asked to run a command, therefore this popup box\n\
tells that you can:\n\
 o Type a command, then press EXECUTE;\n\
 o Press EXECUTE to accept the default command listed in parentheses; or\n\
 o Press CANCEL to remove the popup box without executing a command.\n\
\n\
Press CANCEL to continue."
$
P_NOCMD "Specify a command.#1"

H_P_NOCMD "0611-380 The Print Helper program cannot interpret the command that\n\
you ask it to run.  Select the option to specify a command, then enter a\n\
command name understood by the the Print Helper program.\n\
\n\
Press CANCEL to continue."
$
P_FILE "Type the file for printing (the default is %s):                  #1"
$ Please include the traling spaces in this message.  The exact number of
$ spaces is not critical.

H_P_FILE "You are asking to print the currently displayed data into a file.\n\
This popup box asks for the file name.  Either accept the default name\n\
that is in the parentheses, or specify a new file name.  You can:\n\
 o Press EXECUTE to accept the default file name;\n\
 o Type a file name, then press EXECUTE; or\n\
 o Press CANCEL to remove the popup box.\n\
\n\
Press CANCEL to continue."
$
P_MAKEFILE "The specified file %s does not now exist, but you can create it\n\
by pressing EXECUTE.\n\
Press CANCEL to return to editing, or press HELP for more information.#1"

H_P_MAKEFILE "The Print Helper program is asked to print the currently displayed\n\
data into a file.  However, the specified target file does not exist.\n\
This popup box asks if Print Helper should create this file. Either:\n\
  o Press EXECUTE to create the file, or\n\
  o Press CANCEL to remove the popup box without creating the file.\n\
\n\
Press CANCEL to continue."
$
P_OVERWRFILE "Do not write over the file currently being edited.#1"

H_P_OVERWRFILE "0611-381 You asked to print the file currently being edited back to the\n\
same file name.  This is not allowed.\n\
\n\
Press CANCEL to continue."
$
P_CLOBBERFILE "The file %s already exists.\n\
Press EXECUTE to write over this file.\n\
Press CANCEL to return to editing, or press HELP for more information.#1"

H_P_CLOBBERFILE "The Print Helper program is asked to print the contents of the current\n\
file into a file that already exists.  This popup box asks whether to write\n\
over the existing file with the contents of the current file.  Either:\n\
 o Press EXECUTE to write over the file, or\n\
 o Press CANCEL to continue without writing over the file.\n\
\n\
A print option is available that appends data to an existing file\n\
rather than writing over existing data.  Reach this option by pressing\n\
CANCEL to remove the popup box, and then pressing PRINT.\n\
\n\
Press CANCEL to continue."
$
P_ISDELTA "Cannot append print output to a structured file.#1"

H_P_ISDELTA "0611-382 The Print Helper program is asked to append the current file\n\
to a structured file.  This is not allowed.  The current file can only be\n\
appended to a text file.\n\
\n\
Press CANCEL to continue."
$
P_ABSFORM "Internal Error (Cannot get the name of the current form.).#1"
$ Do not translate "cannot get the name of the current form."

H_P_ABSFORM "The Print Helper program cannot determine the name of the form being used.\n\
\n\
Press CANCEL to continue."
$
P_MULTIWIN "Cannot print a screen showing more than one window.#1"

H_P_MULTIWIN "0611-383 PRINT cannot be used when the screen shows more than one window.\n\
Use ENTER WINDOW to remove all windows but the one you wish to print. Now use\n\
PRINT to print this remaining window.\n\
\n\
Press CANCEL to continue."
$
P_NOFORM "Internal Error (Cannot open form %s).#1"
$ Do not translate "couldn't open form %s"

H_P_NOFORM "The Print Helper program cannot access the current form.\n\
\n\
Press CANCEL to continue."
$
P_BADFORM "Internal Error (Error reading form %s.)#1"
$ Do not translate "error reading form %s"

H_P_BADFORM "The Print Helper program cannot read the necessary form.\n\
\n\
Press CANCEL to continue."
$
P_AFILE "Cannot open file %s for appending.#1"

H_P_AFILE "0611-384 The selected Print Menu option appends the contents of\n\
the current file to a specified file.  The Print Helper program\n\
cannot append to the specified file. The file might not have the\n\
correct write permission, or because it might be in a directory without\n\
the correct write permission.\n\
\n\
Press CANCEL to continue."
$
P_OFILE "Cannot open file %s for writing.#1"

H_P_OFILE "0611-385 The selected Print Menu option prints the contents of\n\
the current file to a specified file.  The Print Helper program\n\
cannot write to the specified file. The file might not have the\n\
correct write permission, or because it might be in a directory without\n\
the correct write permission.\n\
\n\
Press CANCEL to continue."
$
P_EXEC "Running %s."
$
P_CONTINUE "\n\
Press RETURN to continue editing:"
$
P_BADFD "Internal Error (Wrong file descriptor fd = %d.)#1"
$ Do not translate "wrong file descriptor fd = %d"

H_P_BADFD "The Print Helper program has encountered a problem.\n\
\n\
Press CANCEL to continue."
$
P_RSTDIN "Cannot open file %s for reading.#1"

H_P_RSTDIN "0611-386 The Print Helper program cannot open the specified file\n\
for reading.  Check path names and permissions.\n\
\n\
Press CANCEL to continue."
$
P_WSTDOUT "Cannot open file %s for writing.#1"

H_P_WSTDOUT "0611-387 The Print Helper program cannot open the specified file.\n\
Check path names and permissions.\n\
\n\
Press CANCEL to continue."
$
P_ERROUT "Internal Error (Cannot open sub-process standard/error output file\n\
%1 for reading).#1"
$ Translate only "Internal Error".

H_P_ERROUT "The Print Helper program cannot open the specified file.\n\
\n\
Press CANCEL to continue."
$
P_OUTPUT "Output of command %s:\n\n"
$ This message must end in \n\n
$
P_BOX "%s#1"

H_P_BOX "0611-388 Either the option to display all output of a command is\n\
selected, or the print command has terminated on an error condition.\n\
If the option to display output was selected, there is no action necessary.\n\
\n\
Press CANCEL to continue."
$
P_MESS "%s"
$
P_EPROMPT "%s                       #1"
$ Please include the trailing spaces in this message.  The exact number of
$ spaces is not critical.

H_P_EPROMPT "The selected Print Menu option needs more information. Either:\n\
 o Type the information, then press EXECUTE;\n\
 o Press EXECUTE to provide a null response; or\n\
 o Press CANCEL to remove the popup box.\n\
\n\
Press CANCEL to continue."
$
P_APPFILE "Cannot append to the file currently being edited.#1"

H_P_APPFILE "0611-389 You asked to print the file currently being edited back to the\n\
same file name.  This is not allowed.\n\
\n\
Press CANCEL to continue."
