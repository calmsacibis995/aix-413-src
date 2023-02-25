$ @(#)93	1.9  src/bos/usr/sbin/format/format.msg, cmdfiles, bos411, 9428A410j 4/11/94 15:43:39
$
$ COMPONENT_NAME: (CMDFILES) commands that deal manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 200 - 229 reserved for this file.
$
$set MS_FORMAT
$ Do not translate format, fdformat

INSERT  "Insert the new media for %s. Press ENTER when ready: "
$ INSERT        % s is replaced with the device name.
$ INSERT        Leave a space at the end. There is no new line.
ENTER   ""
$ ENTER         This is a null message.

INFO    "Formatting: %1$d side(s), %2$d trk/side, %3$d sect/trk\n"
$ INFO          This is an information message; the first insert is either
$ INFO          1 or 2 for single/doublesided, the 2nd insert is number of
$ INFO          tracks, and the 3rd is number of sectors/track.
COMPL   "Format completed.\n"
USAGE "Usage: format [-fl] [-d Device]\n"
USAGE1 "Usage: fdformat [-h] Device\n"

OPNERR "format: 0653-200 Cannot open %1$s: %2$s\n"
$ OPNERR        %1$s is the device, %2$s is an error number.
NOTDSK "format: 0653-201 A failure occurred accessing device %s.\n"
$ NOTDSK        %s is device.
NOPAR "format: 0653-202 Cannot get drive parameters.\n"
NSETPAR "format: 0653-203 Cannot set drive parameters.\n"
FAILED "format: 0653-204 The format operation failed: %s\n"
$ FAILED        Insert is error code
MEMERR "format: 0653-205 There is not enough memory available now.\n"
DSKBAD "format: 0653-206 The media is bad or not compatible with the hardware.\n"
GERR "format: 0653-207 Cannot read media: %s\n"
$ GERR          General I/O error; %s is errno
CONFLICT1 "format: 0653-208 The -l flag cannot be used with the high density device.\n"
CONFLICT2 "fdformat: 0653-209 The -h flag cannot be used with the low density device.\n"
CONFLICT3 "fdformat: 0653-210 Specify the -h flag with the high density device.\n"
