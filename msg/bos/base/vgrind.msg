$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)63	1.1  src/msg/bos/base/vgrind.msg, bosmsg, msg411, GOLD410 2/9/94 09:28:21
$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting Services
$
$ FUNCTIONS:
$
$ ORIGINS: 3,10,27
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
$ messages for vgrind
$

$set MS_VGRIND
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

$quote "
$ Messages for vgrind.sh

1 "vgrind: 1003-501 %s choice must have a parameter.\n"
$ Do not translate %s, refers to choice.
2 "vgrind: 1003-502 Warning the -W option is not supported\n"
$ Messages for vfontedpr
NOENTRY         "vgrind: 1003-503 The %s language is not supported.\n"
CANNOTFIND      "vgrind: 1003-504 Cannot find vgrindefs file %s\n"
$ Do not translate vgrindefs.
$ Messages for vgrindefs
VGENTTOOLONG    "vgrind: 1003-505 Vgrind entry too long\n"
BADVGENT        "vgrind: 1003-506 Bad vgrind entry\n"
INFLOOP         "vgrind: 1003-507 Infinite tc= loop\n"
