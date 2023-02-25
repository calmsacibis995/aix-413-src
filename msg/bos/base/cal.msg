$ ENGLISH_SOURCE_VERSION= 1.8
$ @(#)13	1.1  src/msg/bos/base/cal.msg, bosmsg, msg411, GOLD410 2/9/94 08:47:54
$
$ COMPONENT_NAME: (CMDMISC) miscellaneous commands
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
$set MS_CAL
$quote "
BADARG "cal: 0702-001 Specify month as a number between 1 and 12\n\
\tSpecify year as a number between 1 and 9999.\n"
BADNOARGS "cal: 0702-009 Too many arguments.\n"
USAGE "Usage: cal [[Month] Year]\n\
\tDisplays a calendar.\n"
$ 
LJAN  "January"
LFEB  "February"
LMAR  "March"
LAPR  "April"
LMAY  "May"
LJUN  "June"
LJUL  "July"
LAUG  "August"
LSEP  "September"
LOCT  "October"
LNOV  "November"
LDEC  "December"
$ Names of the months spelled completely out maximium number of letters is 20
SUN   "Sun"
MON   "Mon"
TUE   "Tue"
WED   "Wed"
THU   "Thu"
FRI   "Fri"
SAT   "Sat"
$ The first three letters of the days of the week
