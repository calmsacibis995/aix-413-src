$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)95        1.1  src/msg/bos/base/factor.msg, bosmsg, msg411, GOLD410 2/9/94 08:58:20
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
$set MS_FACTOR
$quote "

USAGE "Usage: factor Number\n\
\tFactors a number.\n"
$ do not translate the command word factor
NOTINT "factor: 0702-002 Specify an integer ranging from 1 to 1.0e14.\n"
$ do not translate factor
