$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)88	1.1  src/msg/bos/base/colrm.msg, bosmsg, msg411, GOLD410 2/9/94 08:52:04
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
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for colrm
$
$quote "
$
$ Message numbers 030 - 037 reserved for this file
$
$set MS_COLRM
$ Do not translate colrm

USAGE "Usage: colrm StartColumn [EndColumn]\n"
$ If only 1 column, "StartColumn",
$ is specified, colrm will remove StartColumn
$ and all columns to the right.
$ E 2
ERROR1 "colrm: 0653-030 The start column must be 1 or greater.\n"
ERROR2 "colrm: 0653-031 The start column cannot be greater than the end column.\n"
