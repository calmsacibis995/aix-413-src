$ @(#)59        1.4 4/25/91 19:29:20
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
1 #Specify an SID with the -r flag.\n\
You must specify an SID when you are listed more than once in the p-file.\n#
$set 2
1 #The specified SID or your user name is not listed in the p-file.\n\
Use the correct user name or SID number.\n\
You may have forgotten to do a get -e or you may have supplied the wrong\n\
SID number.\n#
$set 4
1 #The diff program failed when SCCS tried to create the delta.\n\
Use local problem reporting procedures.\n\
The program that delta uses to determine the difference between the new\n\
delta and the previous delta failed.  The delta command did not create\n\
the new delta.\n#
$set 5
1 #Internal software error in delta/enter().\n\
Use local problem reporting procedures.\n#
$set 6
1 #Internal software error in delta/enter().\n\
Use local problem reporting procedures.\n#
$set 7
1 #Internal software error in delta/enter().\n\
Use local problem reporting procedures.\n#
$set 13
1 #The specified program failed. The system is trying again with\n\
the specified segmentation size.\n\
The program the delta command uses to compare SCCS files failed.\n\
Delta divides the files it compares into segments.\n\
If the segments are too large, delta tries to divide the files\n\
into smaller segments. If this fails, delta terminates\n\
processing of the SCCS file.\n\
Key in the following for more information:\n\
\t\tsccshelp de17\n#
$set 14
1 #The SOH character is in the first postion of the specified line\n\
of the specified file.\n\
Remove this character or precede it with a \\ (backslash).\n\
You cannot begin any line of an SCCS file with the ASCII SOH (Start of\n\
heading) character unless you precede it with a \\ (backslash). The SOH\n\
character has special meaning to SCCS which causes an error.\n#
$set 15
1 #There is more than one outstanding delta.\n\
Specify the SID number that will be created.\n\
Look at the p-file, choose the SID number of the delta to be created,\n\
and specify it with the -r flag. Do not specify the current SID number;\n\
specify the SID number to be created.\n#
$set 16
1 #Use the -y or the -m flag when you specify standard input.\n\
When delta reads from standard input it does not prompt you for\n\
comments or MR (Modification Request) numbers. You must supply comments\n\
with the -y flag. You must use the -m flag to supply an MR number if the -v\n\
flag has been set for the file with the admin command.\n#
$set 17
1 #The SCCS file that the delta command created may occupy a large amount\n\
of file space. You can remove the delta just created and try again\n\
when the system is more lightly loaded (and perhaps more work space\n\
is available). If no subsequent delta has been made, use the following\n\
procedure:\n\
        1. get -k [s.file]\n\
        2. rmdel -rSID s.file\n\
        3. get -e -g [s.file]\n\
        4. delta s.file\n#
