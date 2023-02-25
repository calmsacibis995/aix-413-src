$ ENGLISH_SOURCE_VERSION= 1.2
$ $*@(#)58      1.1  src/msg/bos/base/Hemenu.msg, bosmsg, msg411, GOLD410 2/9/94 08:45:14

$ $*COMPONENT_NAME: (INED) INed Editor
$ $*
$ $*ORIGINS:  27
$ $*
$ $*This module contains IBM CONFIDENTIAL code. -- (IBM
$ $*Confidential Restricted when combined with the aggregated
$ $*modules for this product)                  SOURCE MATERIALS
$ $*(C) COPYRIGHT International Business Machines Corp. 1991
$ $*All Rights Reserved
$ $*
$ $*US Government Users Restricted Rights - Use, duplication or
$ $*disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

$quote "
$len
$set MS_HEMENU

HOME_DIR	"Show your home directory."
EDIT_PROF	"Edit your editor profile."
SHOW_PROF	"Show your profiles directory."
EDIT_ALT	"Edit your alternate file ($ALTFILE)."
$ Do not translate ALTFILE.
DISP_HIST	"Display the history of the current file."
HELP_PROG	"\\_Helper program:\\+"
$  TRANSLATOR:  in this message, please include the sequences \_
$  and \+ exactly as shown.
CHANGE_HELP	"Change the helper program (it is currently $HELPER)."
$ Do not translate HELPER.
FORM_NAME	"\\_Form name:\\+"
$  TRANSLATOR:  in this message, please include the sequences \_
$  and \+ exactly as shown.
CHANGE_FORM	"Change the form (is is currently $FORM)."
$ Do not translate FORM.
DISP_DATE	"Display the current date and time."
SH_COMM		"sh -c \\_Command:\\+"
$  TRANSLATOR:  in this message, please include the sequences \_
$  and \+ exactly as shown.
$ Translate only the word "Command".
RUN_COMM	"Run a command within a box."
FCOMPRESS	"Compress the current file ($FILE)."
$ Do not translate FILE.
RM_TEMP		"Remove temporary files."
ECONT		"echo \"Press CNTL-D to continue editing\"; export PS1; PS1='eshell> '; exec sh"
$ Note:  This line is a shell command.  The punctuation, including quote
$ marks, must be left exactly as it is.
$ Translate only "Press" and  "to continue editing."
EXEC_COMM	"Execute shell commands."
EXIT_EDTR	"Exit the editor."
