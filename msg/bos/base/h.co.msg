$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)39        1.1 2/9/94 09:00:46
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
$quote #
$set 1
1 #Specify an SCCS file with this command.\n\
You supplied a file name that does not begin with s.\n#
$set 3
1 #SCCS files can have only one link or name.\n\
Unlink the file, then create the SCCS file.\n\
The delta program unlinks the old file and then links to the new file.\n#
$set 4
1 #There is a format error on the specified line.\n\
Restore the backup copy.\n\
An SCCS file that is not correctly formatted could result from manual editing,\n\
which is not a good idea, or from some sort of system error.\n#
$set 5
1 #The end of the file was premature.\n\
Make sure that the last line of the file ends with a newline character or\n\
use local problem reporting procedures.\n\
The SCCS file may have been damaged by a system error or an internal error\n\
in an SCCS command. You may need to locate a backup copy.\n#
$set 6
1 #The file is damaged.\n\
Use local problem reporting procedures.\n\
The computed hash count of the input SCCS file did not check with the\n\
stored hash count.  The file may have been damaged since it was last\n\
accessed, or it may have been manually edited.  If you think the data\n\
is correct, run the following to correct the hash count:\n\
admin -z\n#
$set 7
1 #The path name is too long.\n\
You supplied an absurdly long path name.#
$ Tim: Do we have to reference this help message!
$set 8
1 #The SID you specified is not valid.\n\
The specified SID is syntactically not correct.#
$ Tim: Can we not reference this help message.
$set 10
1 #The creation of the newest delta in the SCCS file is later than\n\
the current time.\n\
Make sure that the system date is set correctly or\n\
use local problem reporting procedures.\n\
This message can also occur if the delta was created when the clock was not\n\
set correctly, even though it is set correctly now, or if the delta was made\n\
very recently at a site in a different time zone.\n#
$set 11
1 #The difference between the current date and the creation date of\n\
the newest delta in the SCCS file is greater than 1 year.\n\
Make sure that the system date is set correctly or\n\
use local problem reporting procedures.\n#
$set 12
1 #The 2 limits of a delta range must be in ascending order.\n\
In a delta list, the 2 limits of a range of deltas were not\n\
in ascending order.\n#
$ Note: Tim: Would you prefer the word "increasing" to the word "ascending"?
$set 13
1 #The delta list syntax is not correct.\n\
A list of deltas was syntactically erroneous.  The correct syntax is:\n\
		<list>  ::=  <range> | <list> , <range>\n\
		<range> ::=  <delta> | <delta> - <delta>\n\
		<delta> ::=  <rel> | <rel> . <lev>\n#
$set 14
1 #You are not authorized to make deltas.\n\
Use local problem reporting procedures to place your user name\n\
or group number on the list of users who can make deltas.\n\
To see who is allowed to add deltas, run the following:\n\
\t\tprs -d:UN: file\n#
$set 15
1 #The specified release is less than the lowest allowed release (the floor).\n\
The specified release must be greater than or equal to the lowest\n\
allowed release.\n\
Either the release you specified or the default release is lower than the\n\
lowest allowed release (the floor).\n\
Use the following command to determine the current floor:\n\
\t\tprs -d:FB:\n#
$set 16
1 #The specified release is greater than the highest allowed release (the ceiling).\n\
The specified release must be less than or equal to the highest allowed release.\n\
Either the release you specified or the default release is higher than the\n\
highest allowed release (the ceiling).\n\
Use the following command to determine the current ceiling:\n\
\t\tprs -d:CB:\n#
$set 17
1 #The p-file is damaged.\n\
Edit the p-file or restore a backup copy.\n\
The p-file has been damaged, perhaps by manual editing.\n\
If you do not know how to fix it, remove it and run the following command\n\
to establish it again:\n\
\t\tget -e -g\n#
$set 20
1 #Cannot create another process at this time.\n\
Try again later or\n\
use local problem reporting procedures.\n\
An attempt to create a subprocess failed.\n\
You may already be running too many processes, or the system process table\n\
may be full.\n#
$set 21
1 #too many MRs\n\
You cannot enter so many MRs.#
$ Tim: Can we not reference this help since it doesn't say anything?
$set 23
1 #The SCCS file is locked against editing.\n\
The specified release is locked against editing.\n\
The specified release (or possibly all releases) for the\n\
SCCS file have been locked using the l flag with the admin command.\n\
To see a list of all locked releases, run the following:\n\
\t\tprs -d:LK: s.file\n#
$set 25
1 #An include or exclude conflict begins at the specified line.\n\
The SIDs in the include or exclude list change conflicting lines.\n\
The lines involved may not be correct.\n#
$ Note to Tim: You wanted to check on this.
$set 26
1 #An include or an exclude conflict ends at the specified line.\n\
The SIDs in the include or exclude list change conflicting lines.\n\
The lines preceding this message may not be correct.\n#
$ Note to Tim: You wanted to check on this.
$set 50
1 #Cannot execute the specified program.\n\
Check path name and permissions or\n\
use local problem reporting procedures.\n\
The system could not run the specified program. It may not be executable or\n\
it may have been removed.\n#
