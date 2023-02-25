$ ENGLISH_SOURCE_VERSION= 1.4.1.1
$ @(#)89	1.2  src/msg/bos/base/tc.msg, bosmsg, msg411, 9435C411a 9/1/94 10:45:23

$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting
$
$ FUNCTIONS:tc.msg
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
$set MS_tc        Define initial set#
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

M_MSG_2    "tc: 1005-801 Cannot find /usr/bin/otc\n"
$ Do not translate /usr/bin/otc, refers to the old tc command.
M_MSG_3    "tc: aspect = %g, erase = %d, dbg = %d, keepon = %d\n"
$ Do not translate  aspect = %g, erase = %d, dbg = %d, keepon = %d.
$ the aspect ratio by default is 1.5, erase, dbg and keepon are flags set to
$ either 1 or 0.
M_MSG_4    "tc: olist[%d] is %d; olist[%d] is %d.\n"
$ Do not translate  olist[%d] is %d; olist[%d] is %d
$ the olist refers to a range of page numbers to be printed.
M_MSG_6    "tc: %c%c%c"
$ Do not translate.  (NOT USED)
M_MSG_7    "tc: "
$ Do not translate.
M_MSG_8    " near line %ld\n"
M_MSG_11   "tc: 1005-807 The file descriptor is pointing to stdin.\n"
$ Do not tranlate stdin.
M_MSG_12   "tc: 1005-808 There are not more pages to backup.\n"
M_MSG_13   "tc: 1005-809 The file descriptor is pointing to stdin.\n"
$ Do not tranlate stdin.
M_MSG_14   "Usage tc [-te] [-oList] [-aNumber] [file... | -] [-sNumber]\n"
M_MSG_21   "Usage tc [-te] [-oList] [-aNumber] [file... | -] [-sNumber]\n"
M_MSG_22   "unknown drawing function %s"
M_MSG_23   "unknown input character %o %c"
M_MSG_24   "{ nested too deep"
M_MSG_25   "extra }"
M_MSG_26   "can't open %s"
