$ ENGLISH_SOURCE_VERSION= 1.11
$ @(#)08	1.1  src/msg/bos/base/nl.msg, bosmsg, msg411, GOLD410 2/9/94 09:09:35
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 465 - 479 reserved for this file.
$
$set MS_NL
$ Do not translate nl
HEADER1 "Header: "
$ Do not translate
BODY1 "Body: "
$ Do not translate
FOOTER1 "Footer: "
$ Do not translate
CANTOPEN "nl: 0653-465 Cannot open %s.\n"
REGERR1 "nl: 0653-466 Regular expression error code %d.\n"
REGERR2 "\tThe regular expression is not valid.\n"
OPTINVAL "nl: 0653-467 illegal option: %s\n\
Usage: nl [-b Type] [-d Delimiter1 Delimiter2] [-f Type] [-h Type]\n\
\t[-i Number] [-l Number] [-n Format] [-p] [-s Separator]\n\
\t[-v Number] [-w Number] [File]\n"
SEPTOOLONG "nl: 0653-468 SEPARATOR TOO LONG - PROCESSING TERMINATED\n"
NOMEM "nl: 0653-469 Out of memory.\n"
INVDELIM1 "nl: 0653-470 Invalid 1st delimiter.\n"
MISSARG "nl: 0653-471 The option '%c' requires a parameter.\n"
INVDELIM2 "nl: 0653-472 Invalid 2nd delimiter.\n"
INVALNUMB "nl: 0653-473 Invalid number for option %s.\n"
OUTOFRANGE "nl: 0653-474 Number is out of range for option %c.\n\
\tThe range of the number parameter is from %d to %d.\n"
