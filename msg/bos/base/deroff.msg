$ ENGLISH_SOURCE_VERSION= 1.3
$ @(#)30	1.1  src/msg/bos/base/deroff.msg, bosmsg, msg411, GOLD410 2/9/94 08:54:59

$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting
$
$ FUNCTIONS: deroff.msg
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
$set MS_deroff        Define initial set#
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

M_MSG_1 "Usage: deroff [-w] [-i] [-k] [-p] [-l] [-u] [-m (a e m s l)] [File] ...\n"
M_MSG_2 "deroff: 1005-502 "
M_MSG_3 ""
M_MSG_4 "deroff: 1005-504 There is not enough memory available now.\n"
