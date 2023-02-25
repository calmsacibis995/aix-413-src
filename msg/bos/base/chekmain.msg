$ ENGLISH_SOURCE_VERSION= 1.5.1.1
$ @(#)30	1.2  src/msg/bos/base/chekmain.msg, bosmsg, msg411, 9435A411a 8/25/94 16:02:50

$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting
$
$ FUNCTIONS: chekmain.msg
$
$ ORIGINS: 3,27,28
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989,1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.

$len 
$set MS_chekmain        Define initial set#
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}
$ chekmain
M_MSG_1 "checkmm: 1003-601 Cannot find or open %s.\n"
$ chekmain1
M_MSG_2 "checkmm: 1003-602  Cannot find or open %s.\n"
$  Refers to a file.
$ chekrout 
M_MSG_03  "checkmm: 1003-603  Line %1$5u: The syntax of %2$s is not valid.\n"
M_MSG_04  "checkmm: 1003-604  Line %1$5u: There is more than one %2$s, the first referenced at line %3$d .\n"
M_MSG_05  "checkmm: 1003-605  Line %1$5u: %2$s is after %3$s at line %4$d.\n"
M_MSG_06  "checkmm: 1003-606  Line %5u: The AT must follow AU.\n"
M_MSG_07  "checkmm: 1003-607  Line %1$5u: The syntax of %2$s is not valid.\n"
M_MSG_08  "checkmm: 1003-608  Line %1$5u: There is more than one %2$s, the first referenced at line %3$d.\n"
M_MSG_09  "checkmm: 1003-609  Line %1$5u: There is an unmatched %2$s.\n"
M_MSG_10  "checkmm: 1003-610  Line %5u: LO must not be between WA/WE or IA/IE.\n"
M_MSG_11  "checkmm: 1003-611  Line %5u: There are more than 5 LOs.\n"
M_MSG_12  "checkmm: 1003-612  Line %1$5u: %2$s is after %3$s at line %4$d.\n"
M_MSG_13  "checkmm: 1003-613  Line %1$5u: The %2$s is between %3$s.\n"
M_MSG_14  "checkmm: 1003-614  Line %1$5u: %2$s is missing before %3$s.\n"
M_MSG_15  "checkmm: 1003-615  Line %1$5u: %2$s is ignored in a %3$s.\n"
M_MSG_16  "checkmm: 1003-616  Line %1$5u: There is an extra %2$s.\n"
M_MSG_17  "checkmm: 1003-617  Line %1$5u: The syntax of %2$s is not valid.\n"
M_MSG_18  "checkmm: 1003-618  Line %1$5u: %2$s precedes %3$s.\n"
M_MSG_19  "checkmm: 1003-619  A NS macro is not finished.\n"
M_MSG_20  "checkmm: 1003-620  An AS macro is not finished.\n"
M_MSG_21  "checkmm: 1003-621  There is a missing MT ! \n"
M_MSG_22  "checkmm: 1003-622  There is a missing LT ! \n"
M_MSG_23  "checkmm: 1003-623  A WA macro is not finished.\n"
M_MSG_24  "checkmm: 1003-624  An IA macro is not finished.\n"
$  chekrout1       
M_MSG_25    "checkmm: 1003-625  Line %1$5u: The %2$s is between %3$s.\n"
M_MSG_26    "checkmm: 1003-626  Line %1$5u: %2$s is missing before %3$s.\n"
M_MSG_27    "checkmm: 1003-627  Line %1$5u: Lists are nested %2$d deep\n"
M_MSG_28    "checkmm: 1003-628  Line %1$5u: There is an extra %2$s.\n"
M_MSG_29    "checkmm: 1003-629  Line %1$5u: The syntax of %2$s is not valid.\n"
M_MSG_30    "checkmm: 1003-630  A DS macro is not finished.\n"
M_MSG_31    "checkmm: 1003-631  A BS macro is not finished.\n"
M_MSG_32    "checkmm: 1003-632  An EQ macro is not finished.\n"
M_MSG_33    "checkmm: 1003-633  The %c right delimeter is missing.\n"
M_MSG_34    "checkmm: 1003-634  A FS macro is not finished.\n"
M_MSG_35    "checkmm: 1003-635  A RS macro is not finished.\n"
M_MSG_36    "checkmm: 1003-636  A TS macro is not finished.\n"
M_MSG_37    "checkmm: 1003-637  A TS H macro is not finished.\n"
M_MSG_38    "checkmm: 1003-638  Unfinished \'%s\'\n"
M_MSG_39    "checkmm: ~~~~~   %d lines done  ~~~~~\n"
$ chekl1.l
M_MSG_41 "checkmm: 1003-641  Line %5u: Expecting a RF.\n"
M_MSG_42 "checkmm: 1003-642  Line %5u: LC %d with %d lists active.\n"
M_MSG_43 "checkmm: 1003-643  Line %5u: Only labeled footnotes appear properly in title,\n\t abstract or a table.\n"
M_MSG_44 "checkmm: 1003-644  Line %5u: Missing LE before H for %s.\n"
M_MSG_45 "checkmm: 1003-645  Line %5u: EQ in %c%c \n"
M_MSG_46 "checkmm: 1003-646  Line %5u: EN in %c%c\n"
M_MSG_47 "checkmm: 1003-647  Line %5u: Missing right delimiter %c, at delimiter change\n"
M_MSG_48 "checkmm: 1003-648  Line %5u: Right delimiter %c only.\n"
M_MSG_49 "checkmm: 1003-649  Line %5u: Left delimiter %c in EQ.\n"
M_MSG_50 "checkmm: 1003-650  Line %5u: Left delimiter %c only.\n"
M_MSG_51 "only ND, WA/WE or IA/IE may "
$ Do not translate ND, WA/WE, IA/IE.
