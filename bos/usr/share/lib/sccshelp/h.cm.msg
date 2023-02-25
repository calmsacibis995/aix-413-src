$ @(#)45        1.5 4/25/91 19:28:42
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

$
$quote |
$set 1
1 |The specified flag is not valid.\n\
Select a flag recognized by the command.\n\
You supplied a flag which the command does not recognize.|
$set 2
1 |Use the specified flag only once on the command line.\n\
You supplied a flag more than once on the command line.|
$set 3
1 |Specify the file to process.\n\
You omitted the name of the file to be processed.|
$set 4
1 |You cannot lock the specified file.\n\
Check path name and permissions or\n\
wait until no one is using the file before you can lock it.\n\
You must have write permission in the directory where the SCCS\n\
file resides.  If you do not have write permission, you are not\n\
allowed to create any files, including the lock file, in the\n\
directory.\n\
Someone else may be updating the SCCS file (or the p-file).\n\
Wait until they are finished and try again.\n\
If neither of the above is the reason you cannot lock the file,\n\
follow local problem reporting procedures.|
$set 5
1 |The date and time you specified are not in the correct format.\n\
The correct format is: YY[MM[DD[HH[MM[SS]]]]].\n\
You must specify year, but month, day, hour, minute, and second\n\
are optional.|
$set 6
1 |The file must contain SCCS identification keywords.\n\
Insert one or more SCCS identification keywords into the s.file.\n\
The SCCS file is required to contain at least one identification\n\
keyword. This was specified with admin -fi. You must insert one or\n\
more SCCS identification keywords before you can continue.|
$set 7
1 |There are no SCCS identification keywords in the file.\n\
This is an informational message; it is not an error.\n\
If the command is get, the text of the delta being retrieved contains no\n\
keywords.  If the command is admin or delta, the text provided for the\n\
delta contains no keywords.  If this is unacceptable, do a get -e,\n\
add the keywords, and then delta.|
$set 8
1 |Do not supply a value for the specified flag.\n\
You supplied a value for the flag, but it does not require one.|
$set 9
1 |Internal error. The passwd file is not accessible.\n\
Follow local problem reporting procedures.\n\
For some reason the passwd file is not readable. This is an internal error,\n\
not a user error.|
$set 10
1 |The SCCS file requires one or more specific identification keywords.\n\
Add the keywords to the s.file.\n\
The specific identification keywords specified by admin -fi[String]\n\
were not found. If the command is admin or delta, the text provided\n\
for the delta does not the contain the keywords, which are required.\n\
If the command is get, the delta cannot be extracted because it was entered\n\
without the keywords.  Do a get -e, add the keywords, and then delta.|
$set 20
1 |The SID you specified does not exist.\n\
Check the p-file for existing SID numbers.\n\
The SID you specified is not present in the SCCS file.|
$set 21
1 |Cannot determine the path name of the current directory.\n\
Check permissions on current directory.\n\
If the problem persists, follow local problem reporting procedures.\n\
An attempt to determine the path name of the current directory failed.\n\
Some ancestor of the current directory is not readable or not searchable.|
$set 22
1 |Cannot chdir to the specified directory.\n\
Check path name and permissions or\n\
use local problem reporting procedures.\n\
You cannot reach the specified directory.\n\
One or more segments of the path may be unreadable or unsearchable.|
$set 23
1 |Supply a value for the -r flag that is greater than 0 and\n\
less than 10,000.\n\
You supplied a value for the -r keyletter that was out of range--either\n\
less than 1 or greater than 9,999.|
$set 24
1 |The specified SCCS file does not allow MR numbers.\n\
You specified MRs, but the SCCS file being processed does not allow them.\n\
If you want to allow MRs in the SCCS file, use the -m flag with the\n\
admin command when you create the SCCS file.|
$set 25
1 |Use a valid MR number or numbers.\n\
The MR validation program returned a non-zero status.\n\
This indicates that one or more of the specified MRs is not valid.|
$set 26
1 |Specify an MR number or numbers on the command line or as standard input.\n\
No MRs were specified, but the file being processed requires them.|
