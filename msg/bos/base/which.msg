$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)71        1.1  src/msg/bos/base/which.msg, bosmsg, msg411, GOLD410 2/9/94 09:28:46
$
$ COMPONENT_NAME: (CMDSCAN) commands that scan files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.
$ messages for which
$len
$set MS_WHICH        Define initial set#

$ \tGLOBAL COMMENT FOR TRANSLATORS
$ Do not translate which -- this is the command name.
$ Do not translate %1$s %2$s etc.
$ The program replaces %s,%1$s,etc with a string of
$ characters.
1                    "which: 0652-140 Cannot find %s.\n"
2                    "which: 0652-141 There is no %1$s in %2$s.\n"
3		     "Usage: which CommandName...\n"
