$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)69	1.1  src/msg/bos/base/aclget.msg, bosmsg, msg411, GOLD410 2/9/94 08:45:48
$
$ COMPONENT_NAME:  (CMDSDAC) security: access control
$
$ FUNCTIONS:  acledit
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1994
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ messages for aclget
$ 3002-110 thru 3002-119
$

$set MS_ACLGET

$quote "

USAGE		"Usage: aclget [-o outfile] filename\n"
NOFIND		"3002-110  Cannot find %s\n"
NOBUF		"3002-111  Cannot allocate buffer for ACL\n"
