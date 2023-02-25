$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)49	1.1  src/msg/bos/base/spell.msg, bosmsg, msg411, GOLD410 2/9/94 09:19:44
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
$ messages for spell
$

$set MS_SPELL
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

$quote "

$ messages for spell.sh
1 "spell: 1001-101 The -v choice is not supported on pdp11.\n"
$ Do not translate pdp11.
2 "spell: 1001-102 All but the last options are ignored.\n"
3 "spell: 1001-103 Cannot find or open %s.\n"
$ Do not translate %s, refers to the file.
4 "spell: 1001-104 Cannot identify local spell file.\n"
5 "spell: 1001-105 %s is not a valid flag.\n"
$ Do not translate %s.
6 "Usage: spell [-v] [-b] [-x] [-l] [-i] [+Wordlist] [-d Hashlist] [-s Hashstop] [-h Historylist] [File]\n"
7 "spell: 1001-106 All but the last -i/-l flags are ignored.\n"
$ messages for spellinprg.c
8 "spellin: 1001-002 Cannot find or open the List file %s.\n"
SPIN_ARG "spellin: 1001-001 The parameter count is not correct.\n"
SPIN_WID "spellin: expected code widths = %f\n"
SPIN_ITMS "spellin: %1$ld items, %2$d ignored, %3$d extra, %4$u words occupied\n"
SPIN_BITS "spellin: %1$f table bits/item, %2$f table+index bits\n"

$ messages for spellout.c
ARGCNT     "spellout: 1001-201 The parameter count is not correct.\n"
CANTINIT   "spellout: 1001-202 Cannot initialize the hash table.\n"
