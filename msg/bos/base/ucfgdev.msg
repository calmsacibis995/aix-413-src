$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)23	1.2  src/msg/bos/base/ucfgdev.msg, bosmsg, msg411, GOLD410 4/7/94 12:05:13
$
$   COMPONENT_NAME: DSAUCFGDEV
$
$   FUNCTIONS: Diagnostic message file.
$		
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
$       This file contains the corresponding messages for the menus and
$	errors generated from the UCFGDEV Utility.
$
$ END OF GENERAL NOTES

$quote "
$set GIO_SET

GIO_TITLE "DIALS AND LPFKEYS CONFIGURATION SERVICE AID"
$
$ See GENERAL NOTES on titles

GIO_LIST "\
List All Dials/Lighted Program Function Keyboards"

GIO_ADD_DIAL "Configure (Add) a Dial Device"

GIO_ADD_LPFK "Configure (Add) a LPFK Device"

GIO_RM_DIAL "Unconfigure (Remove) a Dial Device"

GIO_RM_LPFK "Unconfigure (Remove) a LPFK Device"

GIO_LAST "Move cursor to selection, and press Enter."

$	msg set used in display_dev()
$
$set GIO_LST_SET
GIO_LST_TITLE "LIST ALL DIALS/LIGHTED PROGRAM FUNCTION KEYBOARDS\n\n\
The following Dials/LPFKs are in the system.\n"

GIO_LST_LAST "\n\nTo continue, press Enter."

$set GIO_UCFG_SET
GIO_UCFG_TITLE_D "ADD A DIAL DEVICE\n\n\
A tty device is configured on the native serial ports. The dials and\n\
lighted programmed function keyboard cannot be configured on either\n\
native serial port if a tty is already in the configured state."
GIO_UCFG_TITLE_L "ADD A LPFK DEVICE\n\n\
A tty device is configured on the native serial ports. The dials and\n\
lighted programmed function keyboard cannot be configured on either\n\
native serial port if a tty is already in the configured state."

GIO_OPT_NO_UNCFG	"\
Do not unconfigure tty on native serial ports."

GIO_OPT_UNCFG	"\
Unconfigure %s on native serial ports."

GIO_UCFG_LAST "Move cursor to selection, then press Enter.\n"

$set GIO_ADD_SET
GIO_ADD_TITLE_D "CONFIGURE (ADD) A DIAL DEVICE\n\n\
Select a parent adapter from the serial ports listed below.\n"
GIO_ADD_TITLE_L "CONFIGURE (ADD) A LPFK DEVICE\n\n\
Select a parent adapter from the serial ports listed below.\n"
GIO_ADD_LAST "Move cursor to selection, then press Enter.\n"

$set GIO_DEL_SET
GIO_DEL_TITLE_D "UNCONFIGURE (REMOVE) A DIAL DEVICE\n\n\
The following device will be removed:\n\n\
To continue, press Enter."
GIO_DEL_TITLE_L "UNCONFIGURE (REMOVE) A LPFK DEVICE\n\n\
The following device will be removed:\n"
GIO_DEL_LAST "To remove this device. press Enter."

$set ERROR_SET
ODM_ACCESS_ERR	"Error retrieving ODM data from %s class."

UNAVAILABLE	"\
***  WARNING  ***\n\n\
All native serial adapters are already configured\n\
with dials or lighted program function keyboards."

NSIO_NOTCFG	"\
***  FATAL ERROR  ***\n\n\
No native serial adapters are \n\
configured."

CHDEV_ERR	"\
***  WARNING  ***\n\n\
Can't disable login on %1$s.\n\
Error upon return from /usr/sbin/chdev.\n\
Error message received = \n%2$s"

RMDEV_ERR	"\
***  WARNING  ***\n\n\
Can't delete device %1$s.\n\
Error upon return from /usr/sbin/rmdev.\n\
Error message received = \n%2$s"

MKDEV_ERR	"\
***  WARNING  ***\n\n\
Can't add device to %1$s.\n\
Error upon return from /usr/sbin/mkdev.\n\
Error message received = \n%2$s"

$set GIO_CONFIRM_SET
GIO_ADD_CONFIRM	"\
The following device has been configured on\n\n\
 %1$s - %2$s\n\n\
%3$s %4$s %5$s\n\n\
To continue, press Enter."
$
GIO_DEL_CONFIRM	"\
The following device has been removed:\n\n\
%1$s %2$s %3$s\n\n\
To continue, press Enter."
$
$set GIO_RESTORE_SET
GIO_RESTORE "\
The following devices have been unconfigured \n\
before entering the service aid:\n\n\
%1$s\n\n\
To  restore the system back to the original state,\n\
please run the following commands:\n\n\
%2$s"
$
$set GIO_NULL_SET
GIO_NULL_DEVICE "\
There are no Dials/Lighted Program Function \n\
Keyboards configured in the system.\n\n\
To continue, press Enter."
$

