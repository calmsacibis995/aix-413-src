$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)67	1.1  src/msg/bos/base/dprog.msg, bosmsg, msg411, GOLD410 2/9/94 08:56:50

$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting
$
$ FUNCTIONS: dprog.msg
$
$ ORIGINS: 26,27,28
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.

$len 
$set MS_dprog        Define initial set#
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

M_MSG_1   "diction: 1001-301 The specified flag is not known.\n"
$ Note: This must be the first message of this set, since it is referenced
$       by absolute number from shell as well as by label from C.
M_MSG_2   "diction: 1001-302 Cannot find or open the default dictionary.\n"
M_MSG_3 "diction: 1001-303 Cannot find or open %s.\n"
$ Refers to a file.
M_MSG_10 "diction: 1001-304 Cannot find or open %s.\n"
$ Refers to a file.
M_MSG_12 "diction: 1001-305 A wordlist is too large.\n"
M_MSG_13 "diction: 1001-306 Cannot find or open %s.\n"
$ Refers to a file.
M_MSG_14 "file %s: number of lines %ld number of phrases found %ld\n"
M_MSG_15 "number of sentences %ld number of phrases found %ld\n"
M_MSG_16 "totals: number of lines %ld number of phrases found %ld\n"
