$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)58	1.1  src/msg/bos/base/macref.msg, bosmsg, msg411, GOLD410 2/9/94 09:07:06

$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting
$
$ FUNCTIONS: macref.msg
$
$ ORIGINS: 3,27,28
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
$set MS_macref        Define initial set#
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}
$ Do not delete empty messages M_MSG_1 and M_MSG_2.

M_MSG_1 ""
M_MSG_2 ""
M_MSG_3 "macref: 1004-703 Cannot find or open %s\n"
$  refers to the file.
M_MSG_4   "macref: 1004-704 %s is not a regular file.\n"
$  The user is trying to use macref on a directory or device.
M_MSG_5   "macref: 1004-705 Cannot find or open %s.\n"
$  refers to the file.
M_MSG_10 "macref: 1004-706 Cannot find or open %s.\n"
$  refers to the file.
M_MSG_11   "macref: 1004-707 The cross reference file is empty.\n"
M_MSG_21   "Usage: macref [-n] [-s] [-t] [--] File \n"
M_MSG_22   "File:-%s\n\n"
$ lists the name of the input file
M_MSG_23   "\tMacro Table of Contents\n"
$ header for output (\t is tab char)
M_MSG_24   "Cross-reference listing\n"
$ header for output
M_MSG_25   "Symbol Use Statistics\n"
$ header for output
M_MSG_26   "\nNo Macros Defined.\n"
M_MSG_27   "macref: 1004-708 ? type error: [%s]\n"
$ refers to an error (perhaps internal) in a switch table
