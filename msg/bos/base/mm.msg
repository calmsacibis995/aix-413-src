$ ENGLISH_SOURCE_VERSION= 1.7
$ @(#)85	1.1  src/msg/bos/base/mm.msg, bosmsg, msg411, GOLD410 2/9/94 09:08:25
$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting
$
$ FUNCTIONS:
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
$
$ messages for mm/mmt
$

$set MS_MM
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}
$quote "
$ messages for mm.sh

1 "Usage: %s [12] [-e] [-t] [-c] [-E] [-TTerm] Files \n"
2 "%s. 1004-202 Cannot find or open the input file.\n"
3 "Usage: mmt [-a] [-c] [-e] [-p] [-g] [-t] [-TName] [-DTerm] [-z] Files \n"
