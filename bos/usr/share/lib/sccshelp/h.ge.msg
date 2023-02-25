$ @(#)51        1.4 4/25/91 19:28:57
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
$set 3
1 #Do not use the -e flag with the -m flag on the command line.\n\
The -e flag allows the user to edit the SCCS file.\n\
The -m flag writes the appropriate SID number before each line of text\n\
in the g-file.\n#
$set 4
1 #A writable version of the specified file exists. Remove the writable version\n\
or remove the file's write permission.\n\
After you remove the writable version of the file, use the get command with\n\
the -e flag to edit the file.\n\
To obtain a read-only version of the file, use the get command with the\n\
-p flag.\n#
$set 9
1 #The specified delta is already included.\n\
Specify the delta only once with the -i flag.\n\
With the -i flag, you can designate a list of SIDs (deltas) to be included\n\
in the creation of the g-file, as long as you do not specify the same SID more\n\
than once.  Separate SID numbers by commas and indicate ranges with hyphens.\n#
$set 10
1 #The specified delta is already excluded.\n\
Specify the delta only once with the -x flag.\n\
With the -x flag, you can designate a list of SIDs (deltas) to be excluded\n\
from the creation of the g-file, as long as you do not specify the same SID\n\
more than once. Separate SID numbers by commas and indicate ranges with hyphens.\n#
$set 11
1 #There is an internal software error.\n\
Use local problem reporting procedures.\n#
$set 17
1 #Another user currently is editing the specified delta.\n\
Wait until the user completes the delta.\n\
You cannot do a get with the -e flag at this time.\n\
Another user has done a get with the -e flag on the same SCCS file.\n\
Wait until the user makes a delta before you continue work on the file.\n\
\n\
If you receive this message when you are the only user working on the file,\n\
you can regenerate the file to be edited by using the -k flag with the get\n\
command.\n#
$set 18
1 #WARNING: The specified file is being edited.\n\
This is only for your information; it does not interfere with your activity.\n\
Someone else has done a get with the -e flag on the same SCCS file.\n#
$set 19
1 #The specified serial number does not exist.\n\
Check the SCCS file for existing serial numbers.\n\
The serial number you specified with get -a is not present in\n\
the SCCS file.\n#
