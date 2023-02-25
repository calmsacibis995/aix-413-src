$ ENGLISH_SOURCE_VERSION= 1.10.1.5
$ @(#)83        1.2  src/msg/bos/base/xpreview.msg, bosmsg, msg411, 9430C411a 7/23/94 12:27:59
$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting
$
$ FUNCTIONS: xpreview.msg
$
$ ORIGINS: 27
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1990,1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$   messages for xpreview
$set MS_XPREVIEW
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

$quote "

$ Keep translations as brief as possible.  This is a motif application
$ which uses the majority of the message strings as button labels and
$ menu prompts.


$ When translating the following 7 messages maintain the same format
$ of  2 or less words with \n between the words.  Each label shall be
$ no more than 10 characters.  If this limit is too small, do not 
$ translate any of the following 7.
LABEL_NEXT	"Next\nPage"
LABEL_LAST	"Last\nPage"
LABEL_PAGE	"Goto\nPage"
LABEL_FILE	"New\nFile"
LABEL_PRINT1	"Print\nPage"
LABEL_PRINT2	"Print\nFile"
LABEL_QUIT	"Quit"

TITLE_PAGE	"Select Page"
TITLE_FILE	"Select File"

OK		"Ok"
CANCEL		"Cancel"

SELECT1		"Enter the new page number"
SELECT2		"Enter the file name"

EXIT_BTN	"Exit"
WARN_TITLE	"Warning"
 
ENTER_CL	"Enter the Command Line"
ENTER_PP	"Enter the Postprocessor"
$ do not translate postprocessor, this is a key word.
ENTER_PRT	"Enter the Print Command"

SET_CL		"Set Command Line"
SET_PP		"Set Postprocessor"
$ do not translate postprocessor, this is a key word.
SET_PRT		"Set Print Command"

CONF_PR		"Configure Printer"

MSG_IN_BOX	"\
You must first use Mouse Button 3 to set up the command line,\n\
the postprocessor and the desired print command.\0"
$ do not translate postprocessor, this is a key word.


SYNTAX		"\n\
Usage: xpreview [-help] [-page Number] [-backingStore backing-store-type] [file | -]\n" 
FILE_ERR	"xpreview: 1007-002 Warning:  Unable to open file %s\n"

WARNING1	"xpreview: 1007-003 Error occurred while in menu callback\n"
WARNING2	"xpreview: 1007-004 Error occurred while in cancel callback\n"
WARNING3	"xpreview: 1007-005 Error occurred in accept callback\n"

NO_DISPLAY	"xpreview: 1007-006 Can't open display, exiting program.\n"
PRINT_FAILED  	"xpreview: 1007-007 The following command failed:\n%s\n"
NOFILE  	"xpreview: 1007-008 No File Selected"
PRINT_STRING "xpreview: 1007-009 The following command will be executed. \
Status messages may follow.\n%s\n"
PRINT_FAILED2 "xpreview: 1007-010 The command may have failed.\n"
BAD_FILE	"xpreview: 1007-011 The selected file is not in the correct \
format.\n"

PRINT_BTN	"PRINT"
CANCEL_BTN	"CANCEL"
DIRECTORY	"Directories"
FILTER_FILE_TITLE	"File Filter"
FILTER_TITLE	"Filter"
FILE_TITLE	"Files"
SELECT_TITLE	"Selection"
LABEL_PREV	"Previous\nPage"
SET_PRTQ	"Change the Printer Queue"
QUEUE_LIST	"Printer Queues"

DATA_STREAM_ERR	"\
You have requested a printer queue which does not support the\n\
type of data stream your file needs.\n\n\
Select PRINT to continue the printing\n\
Select CANCEL to cancel the print request\n"

MSG_NO_QUE	"xpreview: 1007-012 There are no defined printer queues.\n"
DPS_ERROR	"xpreview: 1007-013 Error while attempting to create a \n\
DPS context.\n"

SEND_TO_SYS	"\
The following command will be executed.  If it appears\n\
incorrect, select CANCEL and use Mouse Button 3 to change the\n\
command line.\n%s\0"

MSG_SETUP	"\
You must first use Mouse Button 3 to set up the command line.\n\
If you are not printing to the default printer queue, use the\n\
Change Printer Queue option to select the desired printer queue.\n"

HELP_CMD	"\
Enter the required troff command line information for\n\
formatting a desired input file.  The input should resemble \n\
the following example.\n\
     pic -Tibm3816 input_file | tbl | troff -man -Tibm3816    \n\n\
When push button Print File or Print Page is selected,\n\
xpreview will add appropriate printing information to this\n\
string and send it to the desired printer queue for printing.\n"

PSDIT 	"xpreview: 1007-014 Error while executing the psdit command.\n"

EXEC_ERR "xpreview: 1007-015 Error while printing the PostScript File.\n"

PIPE_ERR "xpreview: 1007-016 Error with the qprt command.\n"

NO_PSDIT "xpreview: 1007-017 If the psdit command was not found,\n\
please install the TranScript Tools option of the Text Formatting \
Package.\n"

NO_FONT "xpreview: 1007-018 Fonts not found in Registry-Encoding:\n%s\n" 

$ Translator: do not translate psdit, TranScript Tools, or Text
$ Formatting Package.  These are names of packages and a command.

$ the following message is used in DPS/DVI widgets.
NOMEM  		"xpreview: 1007-019 Memory allocation error\n"
NOFONTSET	"xpreview: 1007-020 XCreateFontSet() error\n"
CODESETERR	"xpreview: 1007-021 Locale is different from troff\n"
NO_DPS		"xpreview: 1007-022 The Display PostScript system is not available.\n"

LOAD_DPS1	"xpreview: 1007-023 Error occurred while loading the Display Postscript objects.\nVerify that the Troff Postscript Xpreviewer is installed.\n"
$ Do not translate Troff Postscript Xpreviewer.  It is the name of a package.
LOAD_DPS2	"xpreview: 1007-024 Error binding Dpsshr.o due to insufficient memory.\n"
$ Do not translate Dpsshr.o.  It is the name of a file.
