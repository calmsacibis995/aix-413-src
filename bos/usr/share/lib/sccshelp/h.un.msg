$ @(#)53        1.4 4/25/91 19:29:03
$
$ COMPONENT_NAME: CMDSCCS       source code control system
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

$quote #
$set 1
1 #There is more than one outstanding SID in the p-file.\n\
Specify with the -r flag the new SID you want to unget.\n\
You have two or more entries in the p-file (by executing get -e more than\n\
once for the same SCCS file).  You must use the -r flag to specify the\n\
SID for the new delta you no longer want to make. Be sure to specify the\n\
the new SID that would be created, not the old SID.\n#
$set 2
1 #Your user name is not in the p-file.\n\
Log in with the user name shown in the p-file.\n\
You will also receive this message if you are not the user\n\
who did the get -e of the named file.\n#
$set 3
1 #The specified SID does not exist.\n\
Check the p-file for existing SID numbers.\n\
You have specified an SID with the -r flag that is either not in\n\
the p-file at all, or is in the p-file but is not associated with your user\n\
name. If the SID exists but is not associated with your user name, log in\n\
with the user name associated with the SID.\n\
\nYou may also have forgotten to do a get -e of the named SCCS file.#
