$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)32        1.1 2/9/94 09:00:16
$
$ COMPONENT_NAME: CMDSCCS	source code control system
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
$quote /
$set 1
1 /Specify a header flag with the -d flag.\n\
The -d flag removes the specified header flag from the SCCS file.\n\
You can use the -d flag only with existing SCCS files./
$set 2
1 /Specify a value for the header flag.\n\
The header flag you supplied with the -f flag requires a value.\n\
The correct format is -f{HeaderFlag{Value}}./
$set 3
1 /The header flag you specified is not recognized.\n\
The header flag you supplied with the -d or the -f flag is not correct.\n\
Choose a valid header flag./
$set 4
1 /Do not use the header flag more than once on the command line.\n\
You can specify the -d or -f flag more than once on the same command line.\n\
You cannot, however, specify the same header flag more than once on the\n\
command line./
$set 5
1 /Specify a header flag with the -f flag.\n\
The -f flag places the specified header flag and value (if needed)\n\
in the SCCS file.  You must indicate which header flag you want to\n\
place in the SCCS file./
$set 8
1 /Specify a user name, group name or group number with the -a flag.\n\
The -a flag adds the specified user or group to the list of users who\n\
can make deltas to the SCCS file.  You must, therefore, indicate the\n\
user or group you wish to add to the list of users who can make deltas\n\
to the file./
$set 9
1 /Do not use the -a flag more than 10 times on the command line.\n\
Use the admin command with the -a flag several times if you need to add\n\
more than 10 users or groups to the list of those who can make changes\n\
to the SCCS file./
$set 10
1 /Specify a user name, group name or group number you want to remove.\n\
The -e flag removes the specified user or group from the list of those\n\
allowed to make deltas to the SCCS file. You must specify which user or group\n\
you wish to remove with the -e flag./
$set 11
1 /Do not use the -e flag more than 10 times on the command line.\n\
Use the admin command with the -e flag again if you need to remove\n\
more than 10 users or groups from the list of those who can make changes\n\
to the SCCS file./
$set 12
1 /The header flag does not require a value when used with the -d flag.\n\
The -d flag deletes the specified header flag. A value is not necessary./
$set 13
1 /The header flag does not require a value when used with the -f flag.\n\
The -f flag places the specified header flag in the SCCS file. The header\n\
flag you chose does not require a value./
$set 14
1 /Specify an SID number with the d header flag.\n\
The d header flag allows you to specify the default SID supplied to the get\n\
command.  Use the d header flag with the -f flag to add the default SID\n\
to the SCCS file.  Use the d header flag with the -d flag to remove the\n\
default SID from the SCCS file./
$set 15
1 /Create only one SCCS file at a time with the -i flag.\n\
Use the admin command with the -i flag to create another SCCS file./
$set 16
1 /Use the -r flag only with the -i flag or the -n flag.\n\
The -r flag assigns the release and version number you specify to the initial\n\
SCCS file delta. Because you use this flag only when you create an SCCS\n\
file, you can use the flag only with the -i flag or the -n flag./
$set 17
1 /Specify a file name for the -t flag when you create a new SCCS file.\n\
The -t flag takes descriptive text for the SCCS file from the file specified\n\
with the flag. If you use -t when you create a new SCCS file, you must supply\n\
a file name from which to take the descriptive text./
$set 18
1 /Do not use the -d flag with the -n flag.\n\
The -n flag creates a new, empty SCCS file. The -d flag removes a header flag\n\
from an existing SCCS file.  Since there is no header flag to remove from a\n\
new, empty SCCS file, the two flags are mutually exclusive./
$set 19
1 /The specified file exists. Do not use the -n flag or the -i flag\n\
with an existing file.\n\
You cannot use these flags with an existing file. Both the -n flag and the\n\
-i flag create a new SCCS file./
$set 20
1 /The specified file does not exist. Check path name of existing SCCS file\n\
or use the -n flag or the -i flag to create a new SCCS file.\n\
You cannot modify a file that does not exist./
$set 21
1 /The file contains prohibited data on the specified line.\n\
Remove the SOH ASCII character (binary 001) or precede it\n\
with a \\ (backslash).\n\
You cannot place the SOH (start of heading) ASCII character (binary 001)\n\
at the beginning of a line of an SCCS file unless you quote the SOH with\n\
a \\ (backslash). The SOH character has special meaning to SCCS and causes\n\
an error./
$set 22
1 /Specify a value greater than 0 and less than 10,000\n\
with the f header flag.\n\
The f (floor) header flag makes the number you specify the lowest\n\
release number that a get -e can retrieve./
$set 23
1 /Specify a value greater than 0 and less than 10,000\n\
with the f header flag.\n\
The value you specified for the f (floor) header flag is either less than 1\n\
or greater than or equal to 9999. The f header flag makes the number you\n\
specify the lowest release number that a get -e can retrieve./
$set 24
1 /Specify a value greater than 0 and less than 10,000\n\
with the c header flag.\n\
The c (ceiling) header flag makes the number you specify the highest\n\
release number that a get -e can use./
$set 25
1 /Specify a value greater than 0 and less than 10,000\n\
with the c header flag.\n\
The value you specified for the c (ceiling) flag is either less than 1\n\
or greater than 9999./
$set 26
1 /Name a file, not a directory, with the -i flag.\n\
You specified a directory instead of an SCCS file to name with the\n\
-i flag. You can name one SCCS file only on the command line with the -i flag./
$set 27
1 /The specified version and release must be numeric.\n\
Separate version and release numbers with a comma.\n\
To lock a release against editing, use the following format with the\n\
l header flag: [lNumber[,Number]] .../
$set 28
1 /You cannot lock a version number or a release number greater than 9,999.\n\
At least one of the version or release numbers in your list for locking or\n\
unlocking is greater than 9999./
$set 29
1 /You used the specified flag with the specified directory.\n\
Specify a file, not a directory, with the -i flag or the -t flag.\n\
The -i flag takes the text for a new SCCS file from the specified file.\n\
The -t flag takes descriptive text for the SCCS file from the specified file.\n\
You must, therefore, specify a file, not a directory, with the -i or the\n\
-t flag./
$set 30
1 /Use the -y flag or the -m flag only when you create an SCCS file.\n\
The -y flag inserts comment text into the initial delta.\n\
The -m flag specifies a list of Modification Request (MR) numbers to\n\
be inserted into the SCCS files.\n\
Use the cdc command to change comments or MR numbers of an existing file./
$set 31
1 /Specify a value greater than the lowest release number(floor).\n\
The value you specified for the c (ceiling) flag is less than the\n\
lowest release number(floor). The c header flag makes the number you\n\
specify the highest release number that a get -e can retrieve./
$set 32
1 /Specify a value less than the highest release number(ceiling).\n\
The value you specified for the f (floor) flag is greater than the\n\
highest release number(ceiling). The f header flag makes the number\n\
you specify the lowest release number that a get -e can retrieve./
