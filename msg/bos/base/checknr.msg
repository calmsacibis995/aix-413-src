$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)29	1.1  src/msg/bos/base/checknr.msg, bosmsg, msg411, GOLD410 2/9/94 08:48:43
$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting Services
$
$ FUNCTIONS:
$
$ ORIGINS: 10,26,27
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for checknr
$

$set MS_CHECKNR
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

$quote "

USAGE   "Usage: checknr [-s] [-f] [-a{.Macro1.Macro2...}] [-c{.Command1.Command2...}]\n"
EMPTY   "checknr: 1005-102 Specify a parameter with the -c or -a flag.\n"
TOOLONG "checknr: 1005-103 Macro or directive invalid.\n"
FUNMATCHED  "checknr: 1005-104 There is no match for the \\fP command.\n"
SUNMATCHED  "checknr: 1005-105 There is no match for the \\s0 command.\n"
UNMATCHED   "unmatched "
BUG         "checknr: 1005-107 Bug: stk[%1$d].opno = %2$d = .%3$s, .%4$s\n"
DOESNOTMATCH "does not match %d: \n"
MACUNMATCHED "unmatched .%s\n"
UNKCOM       "the .%s is not valid.\n"
ILLEGAL      "checknr: 1005-111 Not allowed define: %s\n"
COMALLOW     "checknr: 1005-112 Specify a command that is among the %d supported.\n"
