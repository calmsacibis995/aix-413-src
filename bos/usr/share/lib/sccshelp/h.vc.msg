$ @(#)43        1.4 4/25/91 19:28:36
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
1 #The command on the specified line is not recognized.\n\
Specify a valid command.\n\
The line given begins with a control character, but the command\n\
is not a valid one. The vc command expects a valid command after\n\
the control character.\n#
$set 2
1 #The specified keyword name was not used.\n\
The specified keyword name was never used in any replacements.\n\
This is only a warning.\n#
$set 3
1 #The specified keyword was not declared.\n\
The specified keyword was not declared in a :dcl statement.\n\
If you thought you did declare it, check for typographical errors.\n\
This is only a warning.\n#
$set 4
1 #There is an if statement with no matching end statement.\n\
There is a missing end statement, or an extra if statement.\n#
$set 5
1 #The specified keyword is declared twice on the specified line.\n\
Do not declare a keyword more than once on the command line.\n\
The named keyword has been declared again on the indicated line.\n#
$ Note to Tim: This help is so redundant!
$set 6
1 #Do not specify more than 40 keywords on the command line.\n\
A maximum of 40 keywords per invocation of vc are allowed.\n\
This limit has been exceeded.\n#
$ Note to Tim: This help is so redundant!
$set 7
1 #There is a control character on the specified line that is not matched.\n\
Specify an ending control character.\n\
On the designated line there is no control character to indicate the end\n\
of the keyword name.\n#
$set 8
1 #A keyword name on the specified line is too long.\n\
A keyword name cannot exceed 9 characters.\n\
A keyword name exceeds 9 characters on the indicated line.\n#
$ Note to Tim: Can we not reference this help since it is so redundant?
$set 9
1 #A keyword name on the specified line is not valid.\n\
A keyword name must begin with an alphabetic character and must contain\n\
only alphanumeric characters.\n\
The keyword name on the indicated line does not start with an\n\
alphabetic character, or does not contain only alphanumeric characters.\n#
$ Note to Tim: This help is so redundant!
$set 10
1 #There is an end statement without a matching if statement\n\
on the specified line.\n\
There is an extra end statement or a missing if statement.\n#
$ Note to Tim: This help is so redundant!
$set 11
1 #There is a parenthesis error on the specified line.\n\
Specify a parenthesis only where correct.\n\
Specify a right parenthesis for each left parenthesis.\n\
A left parenthesis occurred with no matching right parenthesis\n\
or a parenthesis was found where it was not expected.\n#
$set 12
1 #An operator on the specified line is not valid.\n\
The valid operators are !=, =, <, and >.\n\
The legal operators are !=, =, <, and >.\n#
$ Note to Tim: Do we have to reference this help since it is so redundant?
$set 14
1 #There is a value on the specified line that is not numeric.\n\
You tried to compare a string which contains characters that are not\n\
numeric with another string using the < or the > operator.\n#
$set 15
1 #The :err statement is on the specified line.\n\
The user inserted an :err statement on the specified line.\n#
$set 16
1 #The specified line requires too many replacements.\n\
The maximum number of replacements per line is 32.\n\
Currently, only 32 replacements per line are allowed.\n#
$ Note to Tim: Redundant!
$set 17
1 #The control statement syntax on the specified line is not correct.\n\
Make sure there is an = in all :asg statements.\n\
Make sure there is a right side to all :asg statements.\n\
There is a missing = or the right side of an :asg statement is missing.\n#
$ Note to Tim: Redundant!
$set 18
1 #The if statement syntax on the specified line is not correct.\n\
An & or a | occurred in the wrong place, or was not found when it was\n\
expected.\n#
$set 19
1 #The syntax on the specified line is not correct.\n#
$ Note to Tim: The original message reads "There is something strange about the
$ indicated line." You wanted to look at this further.
$set 20
1 #The specified keyword was not assigned a value.\n\
The specified keyword name was never used in an assignment statement.\n\
This is only a warning.\n#
$set 49
1 # Usage: vc [-a] [-t] [-cCharacter] [-s] [Keyword=Value ...]\n\
\tSubstitutes assigned values in place of keywords.\n#
