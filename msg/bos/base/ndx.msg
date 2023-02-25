$ ENGLISH_SOURCE_VERSION= 1.7
$ @(#)02	1.1  src/msg/bos/base/ndx.msg, bosmsg, msg411, GOLD410 2/9/94 09:09:17
$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting
$
$ FUNCTIONS: ndx.msg
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
$
$ messages for ndx.sh & subj.sh 
$

$set MS_SUBNDX
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}
$quote "
$ messages for ndx.sh & subj.sh


1 "Usage: ndx SubjectFile \"Formatter_CommandLine\"\n"
2 ""
3 "%1$s: 1004-403 Cannot find or open %2$s.\n"
$ First %s is name of program, second %s is input file.
4 "Please check to see that %s exists and is readable.\n"
$ %s is the input file.
5 "ndx: Running\n"
$ Do not translate 'ndx'.
6 "%1$s: 1004-406 %2$s is not a valid formatter.\n"
7 "%s: 1004-407 Valid formatters are troff, nroff, mmt and mm.\n"
$ Do not translate 'troff', 'nroff', 'mmt', or 'mm'.
8 "%1$s: 1004-408 %2$s aborted, files were %3$s %4$s" >&2
$ All %s fields are file names.
9 ""
10 "  Error message from %s.\n"
$ %s is command name
11 ""
12 ""
13 ""
14 "  Please check your formatter options.\n"
15 "ndx: Has completed\n"
$ Do not translate 'ndx'.
16 "subj: 1004-450 Cannot find or open %s.\n"
$ Do not translate 'subj'. %s is file name.
17 ""
18 "subj: Has completed\n"
$ Do not translate 'subj'.
