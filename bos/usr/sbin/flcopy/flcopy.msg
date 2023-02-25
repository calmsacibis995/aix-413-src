$ @(#)04	1.5.2.1  src/bos/usr/sbin/flcopy/flcopy.msg, cmdarch, bos411, 9428A410j 9/13/93 11:19:47
$
$ COMPONENT_NAME: (CMDARCH) archive files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$set MS_FLCOPY
$ Do not translate flcopy

BADT "flcopy: 0511-066 Specify a number of tracks between 1 and 160.\n"

CANTOP "flcopy: 0511-067 Cannot open the file.\n"

CHANGEF "Change the diskette.  Press the Enter key when finished.\n"

FLPPYO "flcopy: 0511-068 Cannot open the device.\n"

HALFT "Starting the copy process of the file to the diskette.\n"

MISSF "flcopy: 0511-069 Specify the file name with the -f flag.\n"
$ This message is followed by the usage message.

USAGE "Usage: flcopy [-h|-r] [-t Number] [-f Device]\n"
