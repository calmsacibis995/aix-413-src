$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)40	1.1  src/msg/bos/base/dgioa.msg, bosmsg, msg411, GOLD410 2/9/94 08:55:32
$
$   COMPONENT_NAME: DAGIO
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1993
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
$ I/O is a technical term and stands for Input/Output.  DO NOT TRANSLATE unless
$   an equivalent term exists in translataing language.
$
$ END OF GENERAL NOTES


$quote "
$set GIO_GENERIC

GIO_TITLE "TESTING GRAPHICS INPUT DEVICE ADAPTER"
$
$ Leave line spacing as shown.  See general notes on length of title.

GIO_TITLE_A "TESTING GRAPHICS INPUT DEVICE ADAPTER IN ADVANCED MODE"
$
$ Leave line spacing as shown.  See general notes on length of title.

GIO_STND_BY "Please stand by."
GIO_PRS_ENT "When finished, press Enter."

$set GIO_WRP_NUM

GIO_WRP_TITLE "TESTING GRAPHICS INPUT DEVICE ADAPTER\n\n\
The following test requires a wrap plug, Part Number 22F9689.\n\n\
Do you have this wrap plug?"
$
$ These lines ask the user if he has a wrap plug IBM part number 22F9689.
$ Display diagnostics menu title and ask the user for a wrap plug. 
$ Leave line spacing as shown.  See general notes on length of title.

GIO_WRP_TITLE_A "TESTING GRAPHICS INPUT DEVICE ADAPTER IN ADVANCED MODE\n\n\
The following test requires a wrap plug, Part Number 22F9689.\n\n\
Do you have this wrap plug?"
$
$ Display diagnostics menu title and ask the user for a wrap plug. 
$ Leave line spacing as shown.  See general notes on length of title.

GIO_WRP_YES "YES"
$ This option is shown when a YES answer is possible.

GIO_WRP_NO  "NO"
$ This option is shown when a NO answer is possible.

GIO_WRP_ACTION "Move cursor to selection, then press Enter."
$ Instruction for users action.

$set GIO_RMV_ATTACHEMENT

GIO_UNPLUG_CBL "\
UNPLUG.........the cable, if attached, from the Graphics I/O Port %1$d."
$
$ Disconnect the cable from the Graiphics I/O port number (1,2).
$ I/O is a technical term.  See general notes.
$ %1$d can be either the number 1 or the number 2.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

GIO_PLUG_WRP "\
PLUG...........the wrap plug (Part Number 22F9689) into the\n\
               Graphics I/O Port %1$d.\n\
               Note:  Do not put the wrap plug into the end of the cable."

$
$ Insert wrap plug into the Graphics I/O port number (1,2).
$ I/O is a technical term.  See general notes.
$ %1$d can be either the number 1 or the number 2.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

$set GIO_RECONNECT

GIO_UNPLUG_WRP "\
UNPLUG.........the wrap plug from the Graphics I/O Port %1$d."
$
$ Remove the wrap plug from the GIO port number (1,2).
$ I/O is a technical term.  See general notes.
$ %1$d can be either the number 1 or the number 2.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

GIO_PLUG_CBL "\
PLUG...........the cable, if removed, into the Graphics I/O Port %1$d."
$
$ Reconnect the cable to GIO port number (1,2), if removed.
$ I/O is a technical term.  See general notes.
$ %1$d can be either the number 1 or the number 2.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

$set GIO_INLOOP

GIO_INLOOP_TITLE "TESTING GRAPHICS INPUT DEVICE ADAPTER IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ Display diagnostics title and number of times diagnostics passed through
$ and report number of errors found.
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during tests.
$ Leave line spacing as shown.  See general notes on length of title.


LPP_DEVICE_CANNOT_UNCONFIGURED "819032 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s         Graphics Input Adapter"


LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE "819033 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set to DIAGNOSE\n\
state.\n\n\
The resource is\n\
- %1$s             %2$s         Graphics Input Adapter"


LPP_DEVICE_CANNOT_SET_TO_DEFINE "819034 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set from DIAGNOSE\n\
state to DEFINE state.\n\n\
The resource is\n\
- %1$s             %2$s         Graphics Input Adapter"

DIAG_DEVICE_CANNOT_CONFIGURED "819035 \
THE DIAGNOSTICS DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s             %2$s         Graphics Input Adapter"


DEVICE_DOES_NOT_EXIST "819036 \
THE SYSTEM INDICATES THAT THE DEVICE DOES NOT EXIST. \n\
No trouble was found.  However, the because the system indicated that \n\
the resource does not exist.\n\n\
The resource is\n\
- %1$s             %2$s         Graphics Input Adapter"

DIAG_DEVICE_CANNOT_UNCONFIGURED "819037 \
THE DIAGNOSTICS DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s         Graphics Input Adapter"


LPP_DEVICE_CANNOT_CONFIGURED "819038 \
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s             %2$s         Graphics Input Adapter"

LPFK_LPP_DEVICE_CANNOT_CONFIGURED "819038 \
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s                      Lighted Program Function Keyboard" 

DIALS_LPP_DEVICE_CANNOT_CONFIGURED "819038 \
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s                      Dials" 

$set GIO_FRUS
$ ---------------------------------------------------------------
$ Reason code set used by device Graphics Input / Output

DGIO_DS_MSG "Graphics application input adapter"
DGIO_REGISTER  "Register test failed."
DGIO_FUSE  "Testing indicates fuse has failed."
DGIO_VPD  "VPD verification test failed."
$ VPD stands for Vital Product Data.
DGIO__COM  "Wrap test failed."
DGIO__MODEM  "Control line wrap test failed."
DGIO__BAUD  "Baud rate test failed."
DGIO__PARITY  "Parity test failed."
DGIO__FIFO  "FIFO test failed."
$ FIFO stands for First In, First Out.
DGIO__COM_DRIVER  "The communication drivers failed."
DGIO__RESET_LINE  "Selective reset line driver failed."
DGIO_ELA  "Error log analysis indicates an adapter failure."
DGIO_CONFIG_FAIL "Device configuration fails."
DGIO_SOFTWARE_ERROR "Software."
DGIO_OPEN_ERROR "Device driver indicates a hardware failure."
DGIO_IOPLANAR "I/O Planar."


