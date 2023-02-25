$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)25	1.1  src/msg/bos/base/showrem.msg, bosmsg, msg411, GOLD410 2/9/94 09:17:11
$
$ COMPONENT_NAME: (INED) INed Editor
$ 
$ ORIGINS:  27
$ 
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1990
$ All Rights Reserved
$ 
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$set MS_SHOWREM 
$
$ Translator:  do not translate anthing that looks like %s or %d or
$ %1$s or %2$d etc. Do not translate showrem
$
M_USAGE "Usage: showrem [-d]"
$
M_OPEN "showrem: 0611-242 Cannot open file %s."
$
M_HOME "showrem: 0611-243 Cannot evaluate the $HOME variable.\n\
\tSet the HOME shell variable and export it."
$ Do not translate HOME, $HOME.
$
M_MALLOC "showrem: 0611-244 Cannot allocate memory in the makename routine."
$ Do not translate "makename".
$
M_LOCK "showrem: 0611-245 Cannot open lock file %s."
$ Do not translate lock
$
M_UNLOCK "showrem: 0611-246 Cannot delete file %s."
