$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)05	1.1  src/msg/bos/base/last.msg, bosmsg, msg411, GOLD410 2/9/94 09:04:33
$
$ COMPONENT_NAME: (CMDSTAT) last
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$set MS_LAST

M_LOGGEDIN      "  still logged in.\n"
M_DOWN          "System is halted by system administrator. "
$ Leave a space at the end, and no new line.
M_CRASH         "System halted abnormally. "
$ Leave a space at the end, and no new line.
M_BEGIN         "\nwtmp begins %10.10s %5.5s \n"
$ Do not translate wtmp, %10.10s, %5.5s
M_INTR          "\ninterrupted %10.10s %5.5s \n"
$ Do not translate  %10.10s, %5.5s
