$ ENGLISH_SOURCE_VERSION= 1.3
$ @(#)07	1.1  src/msg/bos/base/cw.msg, bosmsg, msg411, GOLD410 2/9/94 08:53:21

$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting
$
$ FUNCTIONS: cw.msg
$
$ ORIGINS: 3,27,28
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
$quote "         define quote character.

$len 
$set MS_cw        Define initial set#
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

M_MSG_1  "cw: 1005-201 An UNGETC error has occurred.\n"
M_MSG_3  "'\\\"  ldel = %1$s, rdel = %2$s, font = %3$c, tflag = %4$d\n"
$ Do not translate   
$ ldel and rdel refers to left and right delimiters. tflag refers to transparent
$ mode on or off.
M_MSG_4  "cw: 1005-203 Cannot find or open %s.\n"
$ Refers to the file.
M_MSG_7 "cw: 1005-204 Warning, extraneous blanks may have been deleted.\n"
M_MSG_16 "'\\\"  ldel = %1$s, rdel = %2$s, font = %3$c, tflag = %4$d\n"
$ Do not translate  
$ ldel and rdel refers to left and right delimiters. tflag refers to transparent
$ mode on or off.
M_MSG_24 "'\\\"  ldel = %1$s, rdel = %2$s, font = %3$c, tflag = %4$d\n"
$ Do not translate  
$ ldel and rdel refers to left and right delimiters. tflag refers to transparent
$ mode on or off.
M_MSG_27 "'\\\"  ldel = %1$s, rdel = %2$s, font = %3$c, tflag = %4$d\n"
$ Do not translate   
$ ldel and rdel refers to left and right delimiters. tflag refers to transparent
$ mode on or off.
