$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)49	1.2  src/msg/bos/base/ptx.msg, bosmsg, msg411, 9430C411a 7/23/94 12:26:11

$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting
$
$ FUNCTIONS: ptx.msg
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
$set MS_ptx        Define initial set#
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}
$ Do not add newline characters to these messages.
M_MSG_2     "ptx: 1004-901 The width cannot be zero."
M_MSG_3     "ptx: 1004-902 The width cannot be greater than 200 characters."
M_MSG_4     "ptx: 1004-903 Specify one of these flags -o or -i."
M_MSG_5     "ptx: 1004-904 Specify one of these flags -o or -i"
M_MSG_6     "Usage ptx [-f] [-t] [-r] [-w Number] [-g Number] \n[-o Only|-i Ignore] [-b Breakfile] [File1] [File2]"
M_MSG_7_OLD     "ptx: 1004-906 Specify no more than 3 filenames."
$ No longer used.  Replaced by M_MSG_7 at bottom of file.
M_MSG_8     "ptx: 1004-907 Cannot find or open break character file."
$ Do not translate break character
M_MSG_9     "ptx: 1004-908 There is not enough memory now."
M_MSG_10    "ptx: 1004-909 Cannot find or open file."
M_MSG_11    "ptx: 1004-910 There are too many words."
M_MSG_12    "ptx: 1004-911 There are too many words in the file."
M_MSG_13    "ptx: 1004-912 Cannot find or open sorting file"
M_MSG_14    "ptx: 1004-913 Cannot find or open data file"
M_MSG_15    "ptx: 1004-914 Cannot find or open output file"
M_MSG_16    "ptx: 1004-915 Cannot fork"
M_MSG_18    "ptx: 1004-916 Cannot find or open sorting file"
M_MSG_7     "ptx: 1004-906 Specify no more than 2 filenames."
$ This message replaces M_MSG_7_OLD
